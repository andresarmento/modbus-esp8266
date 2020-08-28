/*
    Modbus Library for Arduino
    ModbusTCP for ESP8266/ESP32
    Copyright (C) 2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/

#pragma once
#include <WiFiClient.h>
#include <WiFiServer.h>
#include "ModbusAPI.h"
#include "ModbusTCPTemplate.h"

class ModbusTCP : public ModbusAPI<ModbusTCPTemplate<WiFiServer, WiFiClient>> {
    private:
    static IPAddress resolver (const char* host) {
        IPAddress remote_addr;
        if (WiFi.hostByName(host, remote_addr))
                return remote_addr;
        return IPADDR_NONE;
    }
    public:
    ModbusTCP() : ModbusAPI() {
        resolve = resolver;
    }
};
