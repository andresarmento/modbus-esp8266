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
		client[i] = NULL;
}

IPAddress ModbusIP::eventSource() {		// Returns IP of current processing client query
	if (n >= 0 && n < MODBUSIP_MAX_CLIENTS && client[n])
		return client[n]->remoteIP();
	return INADDR_NONE;
}

void ModbusIP::task() {
	uint8_t i;
	clientsCleanup();
	while (server->hasClient()) {
		WiFiClient* currentClient = new WiFiClient(server->available());
		if (currentClient != NULL && currentClient->connected()) {
			if (cbConnect == NULL || cbConnect(currentClient->remoteIP())) {
				n = getFreeClient();
				if (n > -1) {
					client[n] = currentClient;
				//for (n = 0; n < MODBUSIP_MAX_CLIENTS; n++) {
				//	if (client[n] == NULL) {
				//		client[n] = currentClient;
				//		break; // for
				//	}
				//}
				//if (n < MODBUSIP_MAX_CLIENTS)	// If client added process next
					continue; // while
				}
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
//		if (!client[n]->connected()) {
//			delete client[n];
//			client[n] = NULL;
//			continue;	// for (n)
//		}
		if (client[n]->available() > sizeof(_MBAP)) {
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
					if (client[i]->readBytes(_frame, _len) < _len) {	//Try to read MODBUS frame
						exceptionResponse((FunctionCode)_frame[0], EX_ILLEGAL_VALUE);
						client[i]->flush();
					} else {
						if (client[i]->localPort() == MODBUSIP_PORT) {
							slavePDU(_frame);	// Slave
						} else {
							for (uint8_t c = 0; c < _len; c++) {
			Serial.print(_frame[c], HEX);
			Serial.print(" ");
							}
							_reply = 0;
							TTransaction* trans = searchTransaction(__bswap_16(_MBAP.transactionId));
							if (trans) {
								if ((_frame[0] & 0x7F) == trans->_frame[0]) {
									masterPDU(_frame, trans->_frame); // Master
								} else {
									_reply = EX_UNEXPECTED_RESPONSE;
								}
								if (cbEnabled && trans->cb) {
									trans->cb(_reply, client[i]->remoteIP());
								}
								free(trans->_frame);
								_trans.remove(*trans);
							}
						}
						client[n]->flush();			// Not sure if we need flush rest of data available
					}
				}
			}
			if (client[i]->localPort() != MODBUSIP_PORT) _reply = REPLY_OFF;
			if (_reply != REPLY_OFF) {
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

void ModbusIP::onConnect(cbModbusConnect cb = NULL) {
	cbConnect = cb;
}