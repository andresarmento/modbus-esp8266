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
#endif

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT 1000
#define MODBUSIP_UNIT	  255
#define MODBUSIP_MAX_TRANSACIONS 16
#define MODBUSIP_MAX_CLIENTS	  4
#define MODBUSIP_ADD_REG  1

// Callback function Type
typedef bool (*cbModbusConnect)(IPAddress ip);

typedef bool (*cbTransaction)(Modbus::ResultCode event, uint16_t transactionId, void* data);

typedef struct TTransaction {
	uint16_t	transactionId;
	uint32_t	timestamp;
	cbTransaction cb = nullptr;
	uint8_t*	_frame = nullptr;
	void*		data = nullptr;
	TAddress	startreg;
    bool operator ==(const TTransaction &obj) const {
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
	std::vector<TTransaction> _trans;
	int16_t		transactionId = 0;  // Last started transaction. Increments on unsuccessful transaction start too.
	int8_t n = -1;

	TTransaction* searchTransaction(uint16_t id);
	void cleanup(); 	// Free clients if not connected and remove timedout transactions
	int8_t getFreeClient();    // Returns free slot position
	int8_t getSlave(IPAddress ip);
	int8_t getMaster(IPAddress ip);
	uint16_t send(IPAddress ip, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, void* data = nullptr, bool waitResponse = true);

	public:
	ModbusIP();
	bool isTransaction(uint16_t id);
	bool isConnected(IPAddress ip);
	bool connect(IPAddress ip);
	bool disconnect(IPAddress addr) {}  // Not implemented yet
	void slave();
	void master();
	void task();
	void begin(); 	// Depricated
	uint16_t transactions();
	void onConnect(cbModbusConnect cb = nullptr);
	void onDisconnect(cbModbusConnect cb = nullptr);
	IPAddress eventSource();

    uint16_t writeCoil(IPAddress ip, uint16_t offset, bool value, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t writeHreg(IPAddress ip, uint16_t offset, uint16_t value, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t writeCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t writeHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readIsts(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readIreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);

	uint16_t pushCoil(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullCoil(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullIsts(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pushHreg(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullHreg(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullIreg(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);

	uint16_t pullHregToIreg(IPAddress ip, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullCoilToIsts(IPAddress ip, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pushIstsToCoil(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pushIregToHreg(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
};