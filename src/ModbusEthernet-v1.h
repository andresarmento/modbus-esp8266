/*
    Modbus Library for Arduino
    ModbusTCP for W5x00 Ethernet
    Copyright (C) 2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/

#pragma once
#include <Dns.h>
#include "ModbusAPI.h"
#include "ModbusTCPTemplate.h"

#undef MODBUSIP_UNIQUE_CLIENTS

class EthernetClientWrapper : public EthernetClient {
    public:
    EthernetClientWrapper(EthernetClient c) : EthernetClient(c) {};
    IPAddress remoteIP() {
        if (connected())
            return IPAddress(0,0,0,1);
        return IPADDR_NONE;
    }
};

class ModbusEthernet : public ModbusAPI<ModbusTCPTemplate<EthernetServer, EthernetClientWrapper>> {
    private:
    static IPAddress resolver (const char* host) {
        DNSClient dns;
        IPAddress ip;
        
        dns.begin(Ethernet.dnsServerIP());
        if (dns.getHostByName(host, ip) == 1)
            return ip;
        else
            return IPADDR_NONE;
    }
    public:
    ModbusEthernet() : ModbusAPI() {
        resolve = resolver;
    }
};
