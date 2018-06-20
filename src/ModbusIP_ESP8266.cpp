/*
    ModbusIP_ESP8266.cpp - ModbusIP Library Implementation
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2018 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "ModbusIP_ESP8266.h"

ModbusIP::ModbusIP() {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		client[i] = nullptr;
}

void ModbusIP::master() {

}

void ModbusIP::slave() {
	server = new WiFiServer(MODBUSIP_PORT);
	server->begin();
}

void ModbusIP::begin() {
	slave();
}

bool ModbusIP::connect(IPAddress ip) {
	//cleanup();
	if(getSlave(ip) != -1)
		return true;
	int8_t p = getFreeClient();
	if (p == -1)
		return false;
	client[p] = new WiFiClient();
	return client[p]->connect(ip, MODBUSIP_PORT);
}

IPAddress ModbusIP::eventSource() {		// Returns IP of current processing client query
	if (n >= 0 && n < MODBUSIP_MAX_CLIENTS && client[n])
		return client[n]->remoteIP();
	return INADDR_NONE;
}

TTransaction* ModbusIP::searchTransaction(uint16_t id) {
   	TTransaction tmp;
	tmp.transactionId = id;
	tmp.timestamp = 0;
	tmp.cb = nullptr;
	tmp._frame = nullptr;
	std::vector<TTransaction>::iterator it = std::find(_trans.begin(), _trans.end(), tmp);
   	if (it != _trans.end()) return &*it;
   	return nullptr;
}


void ModbusIP::task() {
	cleanup();
	while (server->hasClient()) {
		WiFiClient* currentClient = new WiFiClient(server->available());
		if (currentClient == nullptr && !currentClient->connected())
			continue;
		if (cbConnect == nullptr || cbConnect(currentClient->remoteIP())) {
			n = getFreeClient();
			if (n > -1) {
				client[n] = currentClient;
				continue; // while
			}
		}
		// Close connection if callback returns false or MODBUSIP_MAX_CLIENTS reached
		currentClient->flush();
		currentClient->stop();
		delete currentClient;
	}
	for (n = 0; n < MODBUSIP_MAX_CLIENTS; n++) {
		if (client[n] == nullptr)
			continue;	// for (n)
		if (client[n]->available() < sizeof(_MBAP) + 1)
			continue;
		client[n]->readBytes(_MBAP.raw, sizeof(_MBAP.raw));	//Get MBAP
		_len = __bswap_16(_MBAP.length);
		_len--; // Do not count with last byte from MBAP
		
		if (__bswap_16(_MBAP.protocolId) != 0) {   //Check if MODBUSIP packet. __bswap is usless there.
			client[n]->flush();
			continue;	// for (n)
		}
		if (_len > MODBUSIP_MAXFRAME) {	//Length is over MODBUSIP_MAXFRAME
			exceptionResponse((FunctionCode)client[n]->read(), EX_SLAVE_FAILURE);
			client[n]->flush();
		} else {
			_frame = (uint8_t*) malloc(_len);
			if (!_frame) {
				exceptionResponse((FunctionCode)client[n]->read(), EX_SLAVE_FAILURE);
				client[n]->flush();
			} else {
				if (client[n]->readBytes(_frame, _len) < _len) {	//Try to read MODBUS frame
					exceptionResponse((FunctionCode)_frame[0], EX_ILLEGAL_VALUE);
					client[n]->flush();
				} else {
					if (client[n]->localPort() == MODBUSIP_PORT) {
						slavePDU(_frame);	// Slave
					} else {
						_reply = EX_SUCCESS;
						TTransaction* trans = searchTransaction(__bswap_16(_MBAP.transactionId));
						if (trans) {
							if ((_frame[0] & 0x7F) == trans->_frame[0]) {
								masterPDU(_frame, trans->_frame); // Master
							} else {
								_reply = EX_UNEXPECTED_RESPONSE;
							}
							if (cbEnabled && trans->cb) {
								trans->cb((ResultCode)_reply, trans);
							}
							free(trans->_frame);
							//_trans.remove(*trans);
							std::vector<TTransaction>::iterator it = std::find(_trans.begin(), _trans.end(), *trans);
							_trans.erase(it);
						}
					}
					client[n]->flush();			// Not sure if we need flush rest of data available
				}
			}
		}
		if (client[n]->localPort() != MODBUSIP_PORT) _reply = REPLY_OFF;
		if (_reply != REPLY_OFF) {
			_MBAP.length = __bswap_16(_len+1);     //_len+1 for last byte from MBAP
							
			size_t send_len = (uint16_t)_len + sizeof(_MBAP.raw);
			uint8_t sbuf[send_len];				
			memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
			memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
			client[n]->write(sbuf, send_len);
		}
		free(_frame);
		_frame = nullptr;
		_len = 0;
	}
	n = -1;
}

bool ModbusIP::send(IPAddress ip, cbTransaction cb) { // Prepare and send ModbusIP frame. _frame buffer should be filled with Modbus data
#ifdef MODBUSIP_MAX_TRANSACIONS
	if (_trans.size() >= MODBUSIP_MAX_TRANSACIONS)
		return false;
#endif
	int8_t p = getSlave(ip);
	if (p == -1 || !client[p]->connected())
		return false;
	transactionId++;
	_MBAP.transactionId	= __bswap_16(transactionId);
	_MBAP.protocolId	= __bswap_16(0);
	_MBAP.length		= __bswap_16(_len+1);     //_len+1 for last byte from MBAP
	_MBAP.unitId		= MODBUSIP_UNIT;
	size_t send_len = (uint16_t)_len + sizeof(_MBAP.raw);
	uint8_t sbuf[send_len];
	memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
	memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
	if (client[p]->write(sbuf, send_len) != send_len)
		return false;
	TTransaction tmp;
	tmp.transactionId = transactionId;
	tmp.timestamp = millis();
	tmp.cb = cb;
	tmp._frame = _frame;
	_trans.push_back(tmp);
	_frame = nullptr;
	return true;
}


void ModbusIP::onConnect(cbModbusConnect cb) {
	cbConnect = cb;
}

void ModbusIP::onDisconnect(cbModbusConnect cb) {
		cbDisconnect = cb;
}

void ModbusIP::cleanup() { 	// Free clients if not connected and remove timedout transactions
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
		if (client[i] && !client[i]->connected()) {
			IPAddress ip = client[i]->remoteIP();
			delete client[i];
			client[i] = nullptr;
			if (cbDisconnect && cbEnabled) 
				cbDisconnect(ip);
		}
	}
	for (TTransaction& t : _trans) {    // Cleanup transactions on timeout
		if (millis() - t.timestamp > MODBUSIP_TIMEOUT) {
			if (cbEnabled && t.cb)
				t.cb(EX_TIMEOUT, &t);
			std::vector<TTransaction>::iterator it = std::find(_trans.begin(), _trans.end(), t);
			_trans.erase(it);
		}
	}
}

int8_t ModbusIP::getFreeClient() {    // Returns free slot position
	//clientsCleanup();
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		if (!client[i])
			return i;
	return -1;
}

int8_t ModbusIP::getSlave(IPAddress ip) {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		if (client[i] && client[i]->connected() && client[i]->remoteIP() == ip && client[i]->localPort() != MODBUSIP_PORT)
			return i;
	return -1;
}

bool ModbusIP::writeCoil(IPAddress ip, uint16_t offset, bool value, cbTransaction cb) {
	readSlave(COIL(offset), COIL_VAL(value), FC_WRITE_COIL);
	return send(ip, cb);
   }

bool ModbusIP::writeHreg(IPAddress ip, uint16_t offset, uint16_t value, cbTransaction cb) {
	readSlave(HREG(offset), value, FC_WRITE_REG);
	return send(ip, cb);
}

bool ModbusIP::pushCoil(IPAddress ip, uint16_t offset, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x007B)
		return false;
	//addCoil(offset, numregs);	// Should registers requre to be added there or use existing?
	if (numregs == 1) {
		readSlave(COIL(offset), COIL_VAL(Coil(offset)), FC_WRITE_COIL);
	} else {
		writeSlaveBits(COIL(offset), numregs, FC_WRITE_COILS);
	}
	return send(ip, cb);
}

bool ModbusIP::pullCoil(IPAddress ip, uint16_t offset, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x007B)
		return false;
	addCoil(offset, numregs);	// Should registers requre to be added there or use existing?
	readSlave(COIL(offset), numregs, FC_READ_COILS);
	return send(ip, cb);
}

bool ModbusIP::pullIsts(IPAddress ip, uint16_t offset, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x007B)
		return false;
	addIsts(offset, numregs);	// Should registers requre to be added there or use existing?
	readSlave(ISTS(offset), numregs, FC_READ_INPUT_STAT);
	return send(ip, cb);
}

bool ModbusIP::pushHreg(IPAddress ip, uint16_t offset, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x007B)
		return false;
	//addCoil(offset, numregs);	// Should registers requre to be added there or use existing?
	if (numregs == 1) {
		readSlave(HREG(offset), Hreg(offset), FC_WRITE_REG);
	} else {
		writeSlaveWords(HREG(offset), numregs, FC_WRITE_REGS);
	}
	return send(ip, cb);
}

bool ModbusIP::pullHreg(IPAddress ip, uint16_t offset, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x007B)
		return false;
	addHreg(offset, numregs);	// Should registers requre to be added there or use existing?
	readSlave(HREG(offset), numregs, FC_READ_REGS);
	return send(ip, cb);
}

bool ModbusIP::pullIreg(IPAddress ip, uint16_t offset, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x007B)
		return false;
	addIreg(offset, numregs);	// Should registers requre to be added there or use existing?
	readSlave(IREG(offset), numregs, FC_READ_INPUT_REGS);
	return send(ip, cb);
}