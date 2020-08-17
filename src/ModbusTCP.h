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

class ModbusTCP : public ModbusTemplate<IPAddress, ModbusTCPTemplate<WiFiServer, WiFiClient, MODBUSTCP_PORT>> {};
