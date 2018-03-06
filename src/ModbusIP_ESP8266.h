/*
    ModbusIP.h - Header for ModbusIP Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
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
#define MODBUSIP_IDLE 5

// Callback function Type
typedef bool (*cbModbusConnect)(IPAddress ip);

class ModbusMasterIP : public Modbus, public WiFiClient {
	private:
	TRegister* reg;
	TRegister* next;
	uint8_t	   status;
	IPAddress	ip;
	uint32_t	queryStart;
	uint32_t	timeout;
	void connect(IPAddress address);
	public:
	ModbusMasterIP() : WiFiClient() {
	}
	void pushBits(uint16_t address, uint16_t numregs, uint8_t fn);
	void pullBits(uint16_t address, uint16_t numregs, uint8_t fn);
	void pushWords(uint16_t address, uint16_t numregs, uint8_t fn);
	void pullWords(uint16_t address, uint16_t numregs, uint8_t fn);
	void pushCoil();
	void pullCoil();
	void pushIsts();
	void pullIsts();
	void pushHreg();
	void pullHreg();
	void pushIreg();
	void pullIreg();
	public:
	void task();
	uint16_t regGroupsCount();
}

class ModbusIP : public Modbus, public WiFiServer {
    private:
    uint8_t _MBAP[7];
	WiFiClient* client[MODBUSIP_MAX_CLIENTS];
	cbModbusConnect cbConnect = NULL;
	int8_t n = -1;
    public:
	ModbusIP() : WiFiServer(MODBUSIP_PORT) {
	}
	void begin();
    void task();
    void onConnect(cbModbusConnect cb);
    IPAddreess eventSource();
};

#endif //MODBUSIP_ESP8266_H