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
typedef struct TRegisterList {
	TRegister* reg;
	TRegisterList* next;
	uint8_t way;
} TRegisterList;

// Callback function Type
typedef bool (*cbModbusConnect)(IPAddress ip);
class ModbusMasterIP : public Modbus, public WiFiClient {
	private:
	TRegisterList* reg;
	uint32_t   interval;
	uint8_t	   status;
	IPAddress	ip;
	uint32_t	lastChange;
	void connect(IPAddress address);
	public:
	ModbusMasterIP() : WiFiClient() {

	}
	void task();
	void pushBits(uint16_t address, uint16_t numregs);
	void pullBits(uint16_t address, uint16_t numregs;
	void pushWords(uint16_t address, uint16_t numregs);
	void pullWords(uint16_t address, uint16_t numregs;
	void pushCoil();
	void pullCoil();
	void pushIsts();
	void pullIsts();
	void pushHreg();
	void pullHreg();
	void pushIreg();
	void pullIreg();
}
class ModbusIP : public Modbus, public WiFiServer {
    private:
    uint8_t _MBAP[7];
	WiFiClient* client[MODBUSIP_MAX_CLIENTS];
	WiFiClient* server[MODBUSIP_MAX_CLIENTS];
	TRegisterList* _regs[MODBUSIP_MAX_CLIENTS];
	IPAddress _ip[MODBUSIP_MAX_CLIENTS];
	uint32_t    pullMs[MODBUSIP_MAX_CLIENTS];
	uint8_t		status[MODBUSIP_MAX_CLIENTS];
	uint32_t	lastChange[MODBUSIP_MAX_CLIENTS];
	cbModbusConnect cbConnect = NULL;
	WiFiClient* getSlaveConnection(IPAddress address);
	TRegister* searchRegister(uint16_t address, IPAddress from, uint8_t way = MODBUSIP_PULL);
	int8_t n = -1;
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
    IPAddreess eventSource();
};

#endif //MODBUSIP_ESP8266_H