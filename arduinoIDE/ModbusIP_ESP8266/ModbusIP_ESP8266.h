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

class ModbusIP : public Modbus {
    private:
        byte _MBAP[7];
    public:
        ModbusIP();
        void config(const char* ssid, const char* password);
        void task();
};

#endif //MODBUSIP_ESP8266_H

