/*
    ModbusIP_ESP8266.h - Header for ModbusIP Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once

#include <Modbus.h>
#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else
 #include <WiFi.h>
 #include <byteswap.h>
#endif

#ifndef __bswap_16
 #define __bswap_16(num) ((uint16_t)num>>8) | ((uint16_t)num<<8)
#endif

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT   10

#define MODBUSIP_MAX_CLIENTS	    4

// Callback function Type
typedef bool (*cbModbusConnect)(IPAddress ip);
typedef union MBAP_t {
	struct {
	uint16_t transactionId;
	uint16_t protocolId;
	uint16_t length;
	uint8_t	unitId;
	};
	uint8_t raw[7];
};
class ModbusCoreIP : public Modbus {
    protected:
    MBAP_t _MBAP;
	cbModbusConnect cbConnect = NULL;
    public:
    void onConnect(cbModbusConnect cb);
    virtual IPAddress eventSource() {
		
	}
};
typedef struct TTransaction;

class ModbusMasterIP : public ModbusCoreIP, public WiFiClient {
	private:
	std::list<TTransaction> _trans;
	IPAddress	ip;
	uint32_t	timeout;
	uint16_t	transactionId = 0;
	public:
    enum ResultCode {
        RESULT_OK           = 0x01,
        RESULT_TIMEOUT      = 0x02,
		RESULT_DISCONNECT	= 0x03,
		RESULT_ERROR		= 0x04
    };

	ModbusMasterIP() : WiFiClient() {
	}
	void connect(IPAddress address);
	void pushBits(uint16_t address, uint16_t numregs, FunctionCode fn);
	void pullBits(uint16_t address, uint16_t numregs, FunctionCode fn);
	void pushWords(uint16_t address, uint16_t numregs, FunctionCode fn);
	void pullWords(uint16_t address, uint16_t numregs, FunctionCode fn);
	bool send() {
		if (connected()) Serial.println("Connected");
		uint16_t i;
		//MBAP
		_MBAP.transactionId	= __bswap_16(1);
		_MBAP.protocolId	= __bswap_16(0);
		_MBAP.length		= __bswap_16(_len+1);     //_len+1 for last byte from MBAP
		_MBAP.unitId		= 0xFF;
				
		size_t send_len = (uint16_t)_len + sizeof(_MBAP.raw);
		uint8_t sbuf[send_len];
				
//		for (i = 0; i < 7; i++)	    sbuf[i] = _MBAP.raw[i];
//		for (i = 0; i < _len; i++)	sbuf[i+7] = _frame[i];

		memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
		memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
		write(sbuf, send_len);
		for (uint8_t c = 0; c < send_len; c++) {
			Serial.print(sbuf[c], HEX);
			Serial.print(" ");
		}
		Serial.println();
		//Serial.println(_frame[0]);
	}
	bool get() {
		uint8_t i;
		if (!connected()) return false;
		uint16_t raw_len = 0;
		raw_len = available();
		if (available() > sizeof(_MBAP.raw)) {
			readBytes(_MBAP.raw, sizeof(_MBAP.raw));	//Get MBAP

			for (uint8_t c = 0; c < sizeof(_MBAP.raw); c++) {
			Serial.print(_MBAP.raw[c], HEX);
			Serial.print(" ");
			}

			_len = __bswap_16(_MBAP.length);
			_len--; // Do not count with last byte from MBAP
			if (__bswap_16(_MBAP.protocolId) == 0 && _len < MODBUSIP_MAXFRAME) {
				_frame = (uint8_t*) malloc(_len);
				if (_frame) {
					if (readBytes(_frame, _len) == _len) {
						for (uint8_t c = 0; c < _len; c++) {
							Serial.print(_frame[c], HEX);
							Serial.print(" ");
						}
						responcePDU(_frame);
						return true;
					}
				}
			}
			flush();
		}
		return false;
	}
	void pushCoil() {
	}
	void pullCoil(uint16_t offset, uint16_t numregs = 1) {
		readSlave(offset, numregs, FC_READ_COILS);
		send();
	}
	void pullCoils() {
		uint16_t offset;
		uint16_t numregs = 1;
		std::list<TRegister>::iterator it = _regs.begin();
	    //std::vector<TRegister>::iterator it = _regs.begin();
		if (it == _regs.end()) return;
		offset = it->address;
    	while(++it != _regs.end())
    	{
			if (!IS_COIL(it->address)) continue;
        	if (it->address == offset + numregs) {
				numregs++;
				continue;
			}
        	pullCoil(offset, numregs);
			offset = it->address;
			numregs = 1;
    	}
		pullCoil(offset, numregs);
	}
	/*
		TRegister* creg = getHead();
		if (!creg) return;
		while (creg && !IS_COIL(creg->address)) creg = creg->next;
		if (creg) {
			offset = creg->address;
			while (creg->next) {
				creg = creg->next;
				if (IS_COIL(creg->address)) {
					if ((offset + 1) == creg->address) {
						numregs++;
						continue;
					}
					pullCoil(offset, numregs);
					offset = creg->address;
					numregs = 1;
				}
			}
			pullCoil(offset, numregs);
		}
	}
	*/
	void pushIsts() {
	}
	void pullIsts() {
	}
	void pushHreg() {
	}
	void pullHreg() {
	}
	void pushIreg() {
	}
	void pullIreg() {
	}
	public:
	void begin() {
	}
	void task();
	IPAddress eventSource();
	bool pullReg(uint16_t address, uint16_t numregs) {
		addReg(address, numregs);
		return true;
	}
	bool pushReg(uint16_t address, uint16_t numregs);
};

typedef uint16_t (*cbModbusSlave)(TTransaction* query, ModbusMasterIP::ResultCode result);
typedef struct TTransaction {
	uint16_t	transactionId;
	Modbus::FunctionCode fn;
	uint16_t	startreg;
	uint16_t	numregs;
	time_t	timestamp;
	cbModbusSlave cb;
};

class ModbusIP : public ModbusCoreIP, public WiFiServer {
    private:
	WiFiClient* client[MODBUSIP_MAX_CLIENTS];
	std::list<TTransaction> _trans[MODBUSIP_MAX_CLIENTS];
	IPAddress	ip[MODBUSIP_MAX_CLIENTS];

	//cbModbusConnect cbConnect = NULL;
	int8_t n = -1;
    public:
	ModbusIP() : WiFiServer(MODBUSIP_PORT) {
	}
	void begin();
    void task();
    //void onConnect(cbModbusConnect cb);
    IPAddress eventSource();
};