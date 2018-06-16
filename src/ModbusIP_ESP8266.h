/*
    ModbusIP_ESP8266.h - Header for ModbusIP Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2018 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once

#include <Modbus.h>
#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else
 #include <WiFi.h>
 #include <byteswap.h>
#endif

#ifndef __bswap_16
 #define __bswap_16(num) ((uint16_t)num>>8) | ((uint16_t)num<<8)
#endif

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT   10
#define MODBUSIP_UNIT	  255

#define MODBUSIP_MAX_CLIENTS	    4

// Callback function Type
typedef bool (*cbModbusConnect)(IPAddress ip);

typedef struct TTransaction TTransaction;

typedef bool (*cbTransaction)(uint8_t event, IPAddress ip);

typedef struct TTransaction {
	uint16_t	transactionId;
	uint32_t	timestamp;
	cbTransaction cb = nullptr;
	uint8_t*	_frame;
    bool operator ==(const TTransaction &obj) const
	    {
		    return transactionId == obj.transactionId;
	    }
    bool operator >(const TTransaction &obj) const
	    {
		    return timestamp > obj.timestamp;
	    }
};

class ModbusIP : public Modbus {
	public:
		enum EventCode {
			SUCCESS				= 0x00,
    		MASTER_CONNECT		= 0xD1,
    		MASTER_DISCONNECT	= 0xD2,
			TRANSACTION_ERROR	= 0xD4,
			TRANSACTION_TIMEOUT	= 0xD5,
    		SLAVE_CONNECT		= 0xD6,
    		SLAVE_DISCONNECT	= 0xD7
		};
	protected:
	typedef union MBAP_t {
		struct {
			uint16_t transactionId;
			uint16_t protocolId;
			uint16_t length;
			uint8_t	 unitId;
		};
		uint8_t  raw[7];
	};
    MBAP_t _MBAP;
	cbModbusConnect cbConnect = nullptr;
	cbModbusConnect cbDisconnect = nullptr;
	WiFiServer* server;
	WiFiClient* client[MODBUSIP_MAX_CLIENTS];
	std::list<TTransaction> _trans;
	int16_t		transactionId = 0;
	int8_t n = -1;

	TTransaction* searchTransaction(uint16_t id) {
    	TTransaction tmp;
		// = {id, 0, NULL, NULL};
		tmp.transactionId = id;
		tmp.timestamp = 0;
		tmp.cb = nullptr;
		tmp._frame = nullptr;
    	std::list<TTransaction>::iterator it = std::find(_trans.begin(), _trans.end(), tmp);
    	if (it != _trans.end()) return &*it;
    	return nullptr;
	}
	void clientsCleanup() { 	// Free clients if not connected
		for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
			if (client[i] && !client[i]->connected()) {
				IPAddress ip = client[i]->remoteIP();
				delete client[i];
				client[i] = nullptr;
				if (cbDisconnect && cbEnabled) 
					cbDisconnect(ip);
			}	
		}		
	}
	int8_t getFreeClient() {
		//clientsCleanup();
		for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
			if (!client[i])
				return i;
		return -1;
	}

	int8_t getSlave(IPAddress ip) {
		for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
			if (client[i] && client[i]->remoteIP() == ip && client[i]->localPort() != MODBUSIP_PORT)
				return i;
		return -1;
	}

	bool send(IPAddress ip, cbTransaction cb) {
		uint16_t i;
		transactionId++;
		_MBAP.transactionId	= __bswap_16(transactionId);
		_MBAP.protocolId	= __bswap_16(0);
		_MBAP.length		= __bswap_16(_len+1);     //_len+1 for last byte from MBAP
		_MBAP.unitId		= 0xFF;
		size_t send_len = (uint16_t)_len + sizeof(_MBAP.raw);
		uint8_t sbuf[send_len];
		memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
		memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
		int8_t p = getSlave(ip);
		if (p != -1 && client[p]->connected()) {
			if (client[p]->write(sbuf, send_len) == send_len) {
			//if (client[p]->write(_MBAP.raw, sizeof(_MBAP.raw)) == sizeof(_MBAP.raw) && client[p]->write(_frame, _len) == _len) {
				//_trans.push_back({transactionId, millis(), cb, _len, _frame});
				TTransaction tmp;// = {transactionId, millis(), cb, _frame};
				tmp.transactionId = transactionId;
				tmp.timestamp = millis();
				tmp.cb = cb;
				tmp._frame = _frame;
				_trans.push_back(tmp);
				_frame = NULL;
				return true;
			}
/*
			for (uint8_t c = 0; c < send_len; c++) {
				Serial.print(sbuf[c], HEX);
				Serial.print(" ");
			}
			Serial.println();
*/
		}
		return false;
	}

	public:
	uint16_t lastTransaction() {
		return transactionId;
	}
	bool isTransaction(uint16_t id) {
		searchTransaction(id) != nullptr;
	}
	bool isConnected(IPAddress ip) {
		int8_t p = getSlave(ip);
		return  p != -1 && client[p]->connected();
	}

	bool connect(IPAddress ip) {
		if(getSlave(ip) == -1) {
			uint8_t p = getFreeClient();
			if (p != -1) {
				client[p] = new WiFiClient();
				client[p]->connect(ip, MODBUSIP_PORT);
				return true;
			}
		}
		return false;
	}

	bool disconnect(IPAddress addr) {}

	void onDisconnect(cbModbusConnect cb = nullptr) {
		cbDisconnect = cb;
	}

	void slave() {
		begin();
	}

	void master() {
		
	}
	
	ModbusIP() {
	}
	void begin();	// Depricated
    void task();
    void onConnect(cbModbusConnect cb);
    IPAddress eventSource();

	bool pullReg(uint16_t address, uint16_t numregs) {
		addReg(address, numregs);
		return true;
	}
	bool pushReg(uint16_t address, uint16_t numregs);

	public:
	bool pushCoil(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = NULL) {
		//addCoil(offset, numregs);	// Should registers requre to be added there or use existing?
		writeSlaveBits(COIL(offset), numregs, FC_WRITE_COILS);
		return send(ip, cb);
	}
	bool pullCoil(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = NULL) {
		addCoil(offset, numregs);	// Should registers requre to be added there or use existing?
		readSlave(COIL(offset), numregs, FC_READ_COILS);
		return send(ip, cb);
	}
	void pullCoils() {	// Not implemented. Just test code.
		uint16_t offset;
		uint16_t numregs = 1;
		std::list<TRegister>::iterator it = _regs.begin();
		if (it == _regs.end()) return;
		offset = it->address;
    	while(++it != _regs.end())
    	{
			if (!IS_COIL(it->address)) continue;
        	if (it->address == offset + numregs) {
				numregs++;
				continue;
			}
        	pullCoil(offset, numregs);
			offset = it->address;
			numregs = 1;
    	}
		pullCoil(offset, numregs);
	}
	void pushIsts() {
	}
	void pullIsts() {
	}
	void pushHreg() {
	}
	void pullHreg() {
	}
	void pushIreg() {
	}
	void pullIreg() {
	}
};