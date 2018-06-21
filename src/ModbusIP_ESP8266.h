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
#define MODBUSIP_MAX_TRANSACIONS 16
#define MODBUSIP_MAX_CLIENTS	  4

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
	WiFiServer* server = nullptr;
	WiFiClient* client[MODBUSIP_MAX_CLIENTS];
	//std::vector<TTransaction> _trans;
	int16_t		transactionId = 0;  // Last started transaction. Increments on unsuccessful transaction start too.
	int8_t n = -1;

	TTransaction* searchTransaction(uint16_t id);
	void cleanup(); 	// Free clients if not connected and remove timedout transactions
	int8_t getFreeClient();    // Returns free slot position
	int8_t getSlave(IPAddress ip);
	bool send(IPAddress ip, cbTransaction cb);

	public:
	std::vector<TTransaction> _trans;
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

	ModbusIP();
	bool connect(IPAddress ip);
	bool disconnect(IPAddress addr) {}  // Not implemented yet
	void slave();
	void master();
	void task();
	void begin(); 	// Depricated

    void onConnect(cbModbusConnect cb = nullptr);
	void onDisconnect(cbModbusConnect cb = nullptr);
    IPAddress eventSource();

    bool writeCoil(IPAddress ip, uint16_t offset, bool value, cbTransaction cb = nullptr);
	bool writeHreg(IPAddress ip, uint16_t offset, uint16_t value, cbTransaction cb = nullptr);
	bool pushCoil(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr);
	bool pullCoil(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr);
	bool pullIsts(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr);
	bool pushHreg(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr);
	bool pullHreg(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr);
	bool pullIreg(IPAddress ip, uint16_t offset, uint16_t numregs = 1, cbTransaction cb = nullptr);
};