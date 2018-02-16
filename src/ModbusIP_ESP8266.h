/*
    Modbus.h - Header for Modbus Base Library
    Copyright (C) 2014 André Sarmento Barbosa
                  2017 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include <Modbus.h>
#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else
 #include <WiFi.h>
#endif

#ifndef MODBUSIP_ESP8266_H
#define MODBUSIP_ESP8266_H

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT   10

#define MODBUSIP_MAX_CLIENTS	    4

#define MODBUSIP_SLAVE  1
#define MODBUSIP_MASTER 2
#define MODBUSIP_PULL_MS 100
#define MODBUSIP_PUSH 3
#define MODBUSIP_PULL 4
typedef struct TRegisterList {
	TRegister* reg;
	TRegisterList* next;
	uint8_t way;
} TRegisterList;

// Callback function Type
typedef bool (*cbModbusConnect)(IPAddress ip);
class ModbusMaster : public WiFiClient {
	TRegister* pull;
	uint32_t   pullMs;
	uint8_t	   status;
}
class ModbusIP : public Modbus, public WiFiServer {
    private:
    uint8_t _MBAP[7];
	WiFiClient* client[MODBUSIP_MAX_CLIENTS];
	WiFiClient* server[MODBUSIP_MAX_CLIENTS];
	TRegisterList* pull[MODBUSIP_MAX_CLIENTS];
	uint32_t    pullMs[MODBUSIP_MAX_CLIENTS];
	uint8_t	    pullStatus[MODBUSIP_MAX_CLIENTS];
	cbModbusConnect cbConnect = NULL;
	WiFiClient* getSlaveConnection(IPAddress address);
	TRegister* searchRegister(uint16_t address, IPAddress from);
    public:
	ModbusIP() : WiFiServer(MODBUSIP_PORT) {
	}
	void begin(uint8_t mode = MODBUSIP_SLAVE);
	bool pullReg(uint16_t address, IPAddress from, uint32_t interval = MODBUSIP_PULL_MS);
	bool pushReg(uint16_t address, IPAddress to, uint32_t interval = MODBUSIP_PULL_MS);
	bool connectReg(uint16_t address, IPAddress to, uint32_t interval = MODBUSIP_PULL_MS, uint8_t way = MODBUSIP_PULL);
	bool diconnectReg(uint16_t address, IPAddress from);
	bool setPullMs(IPAddress from, uint32_t interval = MODBUSIP_PULL_MS);
	uint8_t getPoolStatus(IPAddress from);
    void task();
    void onConnect(cbModbusConnect cb);
};

#endif //MODBUSIP_ESP8266_H