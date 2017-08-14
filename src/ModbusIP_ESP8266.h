/*
    ModbusIP_ESP8266.h - Header for Modbus IP ESP8266 Library
    Copyright (C) 2015 André Sarmento Barbosa
*/
#include <Modbus.h>
#include <ESP8266WiFi.h>

#ifndef MODBUSIP_ESP8266_H
#define MODBUSIP_ESP8266_H

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT   10

#define TCP_KEEP_ALIVE

class ModbusIP : public Modbus, public WiFiServer {
    private:
        byte _MBAP[7];
	#ifdef TCP_KEEP_ALIVE
	WiFiClient client;
	#endif
    public:
	ModbusIP() : WiFiServer(MODBUSIP_PORT) {
	}
	void begin();
        void task();
};

#endif //MODBUSIP_ESP8266_H

