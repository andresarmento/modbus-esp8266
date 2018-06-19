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
#define MODBUSIP_TIMEOUT 1000
#define MODBUSIP_UNIT	  255
#define MODBUSIP_MAX_TRANSACIONS 32
#define MODBUSIP_MAX_CLIENTS	    4

// Callback function Type
typedef bool (*cbModbusConnect)(IPAddress ip);

typedef struct TTransaction TTransaction;

typedef bool (*cbTransaction)(Modbus::ResultCode event, TTransaction* t);

typedef struct TTransaction {
	uint16_t	transactionId;
	uint32_t	timestamp;
	cbTransaction cb = nullptr;
	uint8_t*	_frame;
    bool operator ==(const TTransaction &obj) const
	    {
		    return transactionId == obj.transactionId;
	    }
};

class ModbusIP : public Modbus {
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
	//std::list<TTransaction> _trans;
	std::vector<TTransaction> _trans;
	int16_t		transactionId = 0;  // Last started transaction. Increments on on unsuccessful transaction start too.
	int8_t n = -1;

	TTransaction* searchTransaction(uint16_t id) {
    	TTransaction tmp;
		tmp.transactionId = id;
		tmp.timestamp = 0;
		tmp.cb = nullptr;
		tmp._frame = nullptr;
    	//std::list<TTransaction>::iterator it = std::find(_trans.begin(), _trans.end(), tmp);
		std::vector<TTransaction>::iterator it = std::find(_trans.begin(), _trans.end(), tmp);
    	if (it != _trans.end()) return &*it;
    	return nullptr;
	}
	void cleanup() { 	// Free clients if not connected and remove timedout transactions
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
				//_trans.remove(t);
				std::vector<TTransaction>::iterator it = std::find(_trans.begin(), _trans.end(), t);
				_trans.erase(it);
			}
		}

	}
	int8_t getFreeClient() {    // Returns free slot position
		//clientsCleanup();
		for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
			if (!client[i])
				return i;
		return -1;
	}

	int8_t getSlave(IPAddress ip) {
		for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
			if (client[i] && client[i]->connected() && client[i]->remoteIP() == ip && client[i]->localPort() != MODBUSIP_PORT)
				return i;
		return -1;
	}

	bool send(IPAddress ip, cbTransaction cb);

	public:
	uint16_t lastTransaction() {
		return transactionId;
	}
	bool isTransaction(uint16_t id) { // Check if transaction is in progress (by ID)
		searchTransaction(id) != nullptr;
	}
	bool isConnected(IPAddress ip) {
		int8_t p = getSlave(ip);
		return  p != -1;// && client[p]->connected();
	}

	bool connect(IPAddress ip) {
		//cleanup();
		if(getSlave(ip) != -1)
			return true;
		int8_t p = getFreeClient();
		if (p == -1)
			return false;
		client[p] = new WiFiClient();
		client[p]->connect(ip, MODBUSIP_PORT);
		if (client[p]->connected()) {Serial.println("Connected");} else Serial.println("Not yet");
		return true;
	}

	bool disconnect(IPAddress addr) {}  // Not implemented yet

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

	public:
    bool writeCoil(IPAddress ip, uint16_t offset, bool value, cbTransaction cb = nullptr) {
		readSlave(COIL(offset), COIL_VAL(value), FC_WRITE_COIL);
		return send(ip, cb);
    }
    bool writeHreg(IPAddress ip, uint16_t offset, uint16_t value, cbTransaction cb = nullptr) {
		readSlave(HREG(offset), value, FC_WRITE_REG);
		return send(ip, cb);
    }
	bool pushCoil(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr) {
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
	bool pullCoil(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr) {
		if (numregs < 0x0001 || numregs > 0x007B)
			return false;
		addCoil(offset, numregs);	// Should registers requre to be added there or use existing?
		readSlave(COIL(offset), numregs, FC_READ_COILS);
		return send(ip, cb);
	}
/*	
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
*/
	bool pullIsts(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr) {
		if (numregs < 0x0001 || numregs > 0x007B)
			return false;
		addIsts(offset, numregs);	// Should registers requre to be added there or use existing?
		readSlave(ISTS(offset), numregs, FC_READ_INPUT_STAT);
		return send(ip, cb);
	}
	bool pushHreg(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr) {
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
	bool pullHreg(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr) {
		if (numregs < 0x0001 || numregs > 0x007B)
			return false;
		addHreg(offset, numregs);	// Should registers requre to be added there or use existing?
		readSlave(HREG(offset), numregs, FC_READ_REGS);
		return send(ip, cb);
	}
	bool pullIreg(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr) {
		if (numregs < 0x0001 || numregs > 0x007B)
			return false;
		addIreg(offset, numregs);	// Should registers requre to be added there or use existing?
		readSlave(IREG(offset), numregs, FC_READ_INPUT_REGS);
		return send(ip, cb);
	}
};