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
			//for (i = 0; i < 7; i++)	_MBAP[i] = client[n]->read(); //Get MBAP
			client[n]->readBytes(_MBAP, sizeof(_MBAP));	//Get MBAP
			_len = _MBAP[4] << 8 | _MBAP[5];
			_len--; // Do not count with last byte from MBAP
		
			if (_MBAP[2] != 0 || _MBAP[3] != 0) {   //Not a MODBUSIP packet
				client[n]->flush();
				continue;	// for (n)
			}
			if (_len > MODBUSIP_MAXFRAME) {	//Length is over MODBUSIP_MAXFRAME
				exceptionResponse((modbusFunctionCode)client[n]->read(), MB_EX_SLAVE_FAILURE);
				client[n]->flush();
			} else {
				_frame = (uint8_t*) malloc(_len);
				if (!_frame) {
					exceptionResponse((modbusFunctionCode)client[n]->read(), MB_EX_SLAVE_FAILURE);
					client[n]->flush();
				} else {
					//for (i = 0; i < _len; i++)	_frame[i] = client[n]->read(); //Get Modbus PDU
					if (client[i]->readBytes(_frame, _len) < _len) {	//Try to read MODBUS frame
						exceptionResponse((modbusFunctionCode)client[n]->read(), MB_EX_ILLEGAL_VALUE);
						client[i]->flush();
					} else {
						this->receivePDU(_frame);
						client[n]->flush();
					}
				}
			}
			if (_reply != MB_REPLY_OFF) {
			    //MBAP
				_MBAP[4] = (_len+1) >> 8;     //_len+1 for last byte from MBAP
				_MBAP[5] = (_len+1) & 0x00FF;
				
				size_t send_len = (uint16_t)_len + 7;
				uint8_t sbuf[send_len];
				
				//for (i = 0; i < 7; i++)	    sbuf[i] = _MBAP[i];
				//for (i = 0; i < _len; i++)	sbuf[i+7] = _frame[i];
				memcpy(sbuf, _MBAP, sizeof(_MBAP));
				memcpy(sbuf + sizeof(_MBAP), _frame, _len);

				client[n]->write(sbuf, send_len);
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
void ModbusMasterIP::pushBits(uint16_t address, uint16_t numregs, modbusFunctionCode fn){
}
void ModbusMasterIP::pullBits(uint16_t address, uint16_t numregs, modbusFunctionCode fn) {
}
void ModbusMasterIP::pushWords(uint16_t address, uint16_t numregs, modbusFunctionCode fn) {
}
void ModbusMasterIP::pullWords(uint16_t address, uint16_t numregs, modbusFunctionCode fn) {
}
void ModbusMasterIP::task() {
}
IPAddress ModbusMasterIP::eventSource() {
}

void ModbusCoreIP::onConnect(cbModbusConnect cb = NULL) {
	cbConnect = cb;
}