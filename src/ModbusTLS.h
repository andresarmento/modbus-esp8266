/*
    Modbus Library for Arduino
    ModbusTLS - ModbusTCP Security for ESP8266
    Copyright (C) 2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/

#include <WiFiClientSecure.h>
#include <WiFiServerSecure.h>

#include "ModbusTCPTemplate.h"
#include "ModbusAPI.h"
// ModbusTLS
class ModbusTLS : public ModbusAPI<IPAddress, ModbusTCPTemplate<WiFiServerSecure, WiFiClientSecure, MODBUSTLS_PORT>> {
	void server(uint16_t port, const char* server_cert = nullptr, const char* private_key = nullptr, const char* ca = nullptr) {};
    //bool connect(IPAddress ip, uint16_t port); Certificate ?
    //bool setCerteficats() {}; ???
};
