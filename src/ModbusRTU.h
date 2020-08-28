/*
    Modbus Library for Arduino
	ModbusRTU
    Copyright (C) 2019-2020 Alexander Emelianov (a.m.emelianov@gmail.com)
	https://github.com/emelianov/modbus-esp8266
	This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/
#pragma once
#include <HardwareSerial.h>
#if defined(ESP8266)
 #include <SoftwareSerial.h>
#endif
#include "ModbusAPI.h"

class ModbusRTUTemplate : public Modbus {
    protected:
        Stream* _port;
        int16_t   _txPin = -1;
		unsigned int _t;	// inter-frame delay in mS
		uint32_t t = 0;		// time sience last data byte arrived
		bool isMaster = false;
		uint8_t  _slaveId;
		uint32_t _timestamp = 0;
		cbTransaction _cb = nullptr;
		void* _data = nullptr;
		uint8_t* _sentFrame = nullptr;
		TAddress _sentReg = COIL(0);
		uint16_t maxRegs = 0x007D;
		#ifdef ESP32
		portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
		#endif

		uint16_t send(uint8_t slaveId, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, void* data = nullptr, bool waitResponse = true);
		// Prepare and send ModbusRTU frame. _frame buffer and _len should be filled with Modbus data
		// slaveId - slave id
		// startreg - first local register to save returned data to (miningless for write to slave operations)
		// cb - transaction callback function
		// data - if not null use buffer to save returned data instead of local registers
		bool rawSend(uint8_t slaveId, uint8_t* frame, uint8_t len);
		bool cleanup(); 	// Free clients if not connected and remove timedout transactions and transaction with forced events
		uint16_t crc16(uint8_t address, uint8_t* frame, uint8_t pdulen);
    public:
		void setBaudrate(uint32_t baud = -1);
	 #if defined(ESP8266)
	 	bool begin(SoftwareSerial* port, int16_t txPin=-1);
	 #endif
		bool begin(HardwareSerial* port, int16_t txPin=-1);
		bool begin(Stream* port);
        void task();
		void master() { isMaster = true; };
		void slave(uint8_t slaveId) {_slaveId = slaveId;};
		uint8_t slave() { return _slaveId; }
		uint32_t eventSource() override {return _slaveId;}
};

class ModbusRTU : public ModbusAPI<ModbusRTUTemplate> {};
