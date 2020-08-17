/*
    Modbus Library for Arduino
    ModbusTCP for W5x00 Ethernet
    Copyright (C) 2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/

#pragma once
#include <Ethernet.h>
#include "ModbusAPI.h"
#include "ModbusTCPTemplate.h"

class ModbusEthernet : public ModbusAPI<IPAddress, ModbusTCPTemplate<EthernetServer, EthernetClient, MODBUSTCP_PORT>> {};