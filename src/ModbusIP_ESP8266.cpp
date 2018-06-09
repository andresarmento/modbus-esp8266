/*
    ModbusIP_ESP8266.cpp - ModbusIP Library Implementation
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "ModbusIP_ESP8266.h"

void ModbusIP::begin() {
	WiFiServer::begin();
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
		client[i] = NULL;
		_trans[i] = NULL;
		ip[i] = INADDR_NONE;
	}
}

IPAddress ModbusIP::eventSource() {		// Returns IP of current processing client query
	if (n >= 0 && n < MODBUSIP_MAX_CLIENTS && client[n])
		return client[n]->remoteIP();
	return INADDR_NONE;
}

void ModbusIP::task() {
	uint8_t i;
	while (hasClient()) {
		WiFiClient* currentClient = new WiFiClient(available());
		if (currentClient != NULL && currentClient->connected()) {
			if (cbConnect == NULL || cbConnect(currentClient->remoteIP())) {
				for (n = 0; n < MODBUSIP_MAX_CLIENTS; n++) {
					if (client[n] == NULL) {
						client[n] = currentClient;
						break; // for
					}
				}
				if (n < MODBUSIP_MAX_CLIENTS)	// If client added process next
					continue; // while
			}
			// If callback returns false or _MAX_CLIENTS reached immediate close connection
			currentClient->flush();
			currentClient->stop();
			delete currentClient;
		}
	}
	for (n = 0; n < MODBUSIP_MAX_CLIENTS; n++) {
		if (client[n] == NULL)
			continue;	// for (n)
		if (!client[n]->connected()) {
			delete client[n];
			client[n] = NULL;
			continue;	// for (n)
		}
		if (client[n]->available() > sizeof(_MBAP)) {
			client[n]->readBytes(_MBAP.raw, sizeof(_MBAP.raw));	//Get MBAP
			_len = __bswap_16(_MBAP.length); //_MBAP.raw[4] << 8 | _MBAP.raw[5];
			_len--; // Do not count with last byte from MBAP
		
			//if (_MBAP.raw[2] != 0 || _MBAP.raw[3] != 0) {   //Not a MODBUSIP packet
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
					if (client[i]->readBytes(_frame, _len) < _len) {	//Try to read MODBUS frame
						exceptionResponse((FunctionCode)_frame[0], EX_ILLEGAL_VALUE);
						client[i]->flush();
					} else {
						this->receivePDU(_frame);
						client[n]->flush();			// Not sure if we need flush rest of data available
					}
				}
			}
			if (_reply != REPLY_OFF) {
			    //MBAP
				_MBAP.length = __bswap_16(_len+1);     //_len+1 for last byte from MBAP
								
				size_t send_len = (uint16_t)_len + sizeof(_MBAP.raw);
				uint8_t sbuf[send_len];
				
				memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
				memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
				client[n]->write(sbuf, send_len);
				// Need to test if we can do not use double buffering
				//client[n]->write(_MBAP.raw, sizeof(_MBAP.raw));;
				//client[n]->write(_frame, _len);
			}
			free(_frame);
			_len = 0;
		}
	}
	n = -1;
}

//void ModbusIP::onConnect(cbModbusConnect cb = NULL) {
//	cbConnect = cb;
//}

void ModbusMasterIP::connect(IPAddress address) {
	WiFiClient::connect(address, MODBUSIP_PORT);
}
void ModbusMasterIP::pushBits(uint16_t address, uint16_t numregs, FunctionCode fn){
}
void ModbusMasterIP::pullBits(uint16_t address, uint16_t numregs, FunctionCode fn) {
}
void ModbusMasterIP::pushWords(uint16_t address, uint16_t numregs, FunctionCode fn) {
}
void ModbusMasterIP::pullWords(uint16_t address, uint16_t numregs, FunctionCode fn) {
}
void ModbusMasterIP::task() {
}
IPAddress ModbusMasterIP::eventSource() {
}

void ModbusCoreIP::onConnect(cbModbusConnect cb = NULL) {
	cbConnect = cb;
}