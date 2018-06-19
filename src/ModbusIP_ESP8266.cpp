/*
    ModbusIP_ESP8266.cpp - ModbusIP Library Implementation
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2018 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "ModbusIP_ESP8266.h"

void ModbusIP::begin() {
	server = new WiFiServer(MODBUSIP_PORT);
	server->begin();
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		client[i] = nullptr;
}

IPAddress ModbusIP::eventSource() {		// Returns IP of current processing client query
	if (n >= 0 && n < MODBUSIP_MAX_CLIENTS && client[n])
		return client[n]->remoteIP();
	return INADDR_NONE;
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
		// If callback returns false or _MAX_CLIENTS reached immediate close connection
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
//							for (uint8_t c = 0; c < _len; c++) {
//			Serial.print(_frame[c], HEX);
//			Serial.print(" ");
//							}
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
							_trans.remove(*trans);
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


void ModbusIP::onConnect(cbModbusConnect cb = nullptr) {
	cbConnect = cb;
}