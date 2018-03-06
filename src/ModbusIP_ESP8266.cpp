/*
    Modbus.h - Header for Modbus Base Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "ModbusIP_ESP8266.h"

void ModbusIP::begin(uint8_t mode) {
	WiFiServer::begin();
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
		client[i] = NULL;
		server[i] = NULL;
		pull[i] = NULL;
		pullMs[i] = MODBUSIP_PULL_MS;
		status[i] = MODBUSIP_IDLE;
	}
}
TRegister* ModbusIP::searchRegister(uint16_t address, IPAddress from, uint8_t way = MODBUSIP_PULL) {
	int8_t i = getSlaveConnection(from);
	if (i == -1) return NULL;
	TRegisterList* root = pull[i];
	while (root) {
		if (root->reg->address == address) return root->reg;
		root = root->next;
	}
	return NULL;
}
// Add ONE register
bool ModbusIP::pullReg(uint16_t address, IPAddress from, uint32_t interval = MODBUSIP_PULL_MS) {
	TRegisterList* reg;
	reg = searchReg(address);
	if (!reg) return false;
	int8_t i = getSlaveConnection(from);
	if (i == -1) {	
		for (i = 0; i < MODBUSIP_MAX_CLIENTS && server[i]; i++) {}
		if (i >= MODBEUSIP_MAX_CLIENTS) return false;	// No free entries
		server[i] = new WiFiClient();
		if (!server[i]) return false;
		status[i] = MODBUSIP_SLAVE_DISCONNECTED;
	}
	if (!pull[i]) {
		pull[i] = malloc(sizeof(TRegisterList));
		if (!pull[i]) return false;
		pull[i]->reg = reg;
		pull[i]->next = NULL;
	}
	TRegisterList* root = pull[i];
	while (root->next) root = root->next;
	if (searchRegister(address, from)) return true;
	root->next = malloc(sizeof(TRegisterList));
	if (!root->next) {
		return false;	//Need to implement cleanup later
	}
	root->reg = reg;
	return true;
}

bool ModbusIP::unpullReg(uint16_t address, IPAddress from) {

}
bool ModbusIP::setPullMs(IPAddress from, uint32_t interval = MODBUSIP_PULL_MS) {

}
uint8_t ModbusIP::getPoolStatus(IPAddress from) {

}
int8_t ModbusIP::getSlaveConnection(IPAddress address) {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
		if (server[i].remoteAddress == address) return i;
	}
	return -1;
}
IPAddreess eventSource() {		// Returns IP of current processing client query
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
			continue;
		if (!client[n]->connected()) {
			delete client[n];
			client[n] = NULL;
			continue;
		}
		uint16_t raw_len = 0;
		raw_len = client[n]->available();
		if (raw_len > 7) {
			for (i = 0; i < 7; i++)	_MBAP[i] = client[n]->read(); //Get MBAP

			_len = _MBAP[4] << 8 | _MBAP[5];
			_len--; // Do not count with last byte from MBAP
			if (_MBAP[2] != 0 || _MBAP[3] != 0) continue;   //Not a MODBUSIP packet
			if (_len > MODBUSIP_MAXFRAME) continue;      //Length is over MODBUSIP_MAXFRAME
			_frame = (uint8_t*) malloc(_len);
			
			raw_len = raw_len - 7;
			for (i = 0; i < _len; i++)	_frame[i] = client[n]->read(); //Get Modbus PDU
			
			this->receivePDU(_frame);
			client[n]->flush();

			if (_reply != MB_REPLY_OFF) {
			    //MBAP
				_MBAP[4] = (_len+1) >> 8;     //_len+1 for last byte from MBAP
				_MBAP[5] = (_len+1) & 0x00FF;
				
				size_t send_len = (uint16_t)_len + 7;
				uint8_t sbuf[send_len];
				
				for (i = 0; i < 7; i++)	    sbuf[i] = _MBAP[i];
				for (i = 0; i < _len; i++)	sbuf[i+7] = _frame[i];

				client[n]->write(sbuf, send_len);
			}
			free(_frame);
			_len = 0;
		}
	}
	n = -1;
}

void ModbusIP::master() {
	for (n = 0; n < MODBUSIP_MAX_CLIENTS; n++) {
		if (_regs[n] == NULL)
			continue;
		if (!server[n]) {
			server[n] = new WiFiClient();
		}
		if (!server[n]->connected()) {
			server[n]->connect(_ip[n], MODBUSIP_PORT);
			continue;
		}
		if (status[n] == MODBUSIP_IDLE) {
			
		}
		uint16_t raw_len = 0;
		raw_len = client[n]->available();
		if (raw_len > 7) {
			for (i = 0; i < 7; i++)	_MBAP[i] = client[n]->read(); //Get MBAP

			_len = _MBAP[4] << 8 | _MBAP[5];
			_len--; // Do not count with last byte from MBAP
			if (_MBAP[2] != 0 || _MBAP[3] != 0) continue;   //Not a MODBUSIP packet
			if (_len > MODBUSIP_MAXFRAME) continue;      //Length is over MODBUSIP_MAXFRAME
			_frame = (uint8_t*) malloc(_len);
			
			raw_len = raw_len - 7;
			for (i = 0; i < _len; i++)	_frame[i] = client[n]->read(); //Get Modbus PDU
			
			this->receivePDU(_frame);
			client[n]->flush();

			if (_reply != MB_REPLY_OFF) {
			    //MBAP
				_MBAP[4] = (_len+1) >> 8;     //_len+1 for last byte from MBAP
				_MBAP[5] = (_len+1) & 0x00FF;
				
				size_t send_len = (uint16_t)_len + 7;
				uint8_t sbuf[send_len];
				
				for (i = 0; i < 7; i++)	    sbuf[i] = _MBAP[i];
				for (i = 0; i < _len; i++)	sbuf[i+7] = _frame[i];

				client[n]->write(sbuf, send_len);
			}
			free(_frame);
			_len = 0;
		}
	}
}

void ModbusIP::onConnect(cbModbusConnect cb = NULL) {
	cbConnect = cb;
}