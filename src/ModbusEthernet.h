/*
    Modbus Library for Arduino
    ModbusTCP for W5x00 Ethernet
    Copyright (C) 2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/

#pragma once
#include <Ethernet.h>
#include "ModbusAPI.h"
#include "ModbusTCPTemplate.h"

#if defined(ethernet_h)
class EthernetClientWrapper : public EthernetClient {
    public:
    EthernetClientWrapper(EthernetClient c) : EthernetClient(c) {};
    IPAddress remoteIP() {
        if (connected())
            return IPAddress(0,0,0,1);
        return IPADDR_NONE;
    }
};
#undef MODBUSIP_UNIQUE_CLIENTS
class ModbusEthernet : public ModbusAPI<IPAddress, ModbusTCPTemplate<EthernetServer, EthernetClientWrapper, MODBUSTCP_PORT>> {};
#else
class ModbusEthernet : public ModbusAPI<IPAddress, ModbusTCPTemplate<EthernetServer, EthernetClient, MODBUSTCP_PORT>> {};
#endif