/*
    ModbusIP_ESP8266.h - Header for ModbusIP Library
    Copyright (C) 2014 André Sarmento Barbosa
                  2017-2018 Alexander Emelianov (a.m.emelianov@gmail.com)
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

// Callback function Type
typedef bool (*cbModbusConnect)(IPAddress ip);

class ModbusIP : public Modbus, public WiFiServer {
    private:
    uint8_t _MBAP[7];
	WiFiClient* client[MODBUSIP_MAX_CLIENTS];
	cbModbusConnect cbConnect = NULL;
    public:
	ModbusIP() : WiFiServer(MODBUSIP_PORT) {
	}
	void begin();
    void task();
    void onConnect(cbModbusConnect cb);
};

#endif //MODBUSIP_ESP8266_H

