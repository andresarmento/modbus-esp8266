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
#endif

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT   10

#define MODBUSIP_MAX_CLIENTS	    4

#define MODBUSIP_SLAVE  1
#define MODBUSIP_MASTER 2
#define MODBUSIP_PULL_MS 100
#define MODBUSIP_PUSH 3
#define MODBUSIP_PULL 4
#define MODBUSIP_IDLE 5

// Callback function Type
typedef bool (*cbModbusConnect)(IPAddress ip);
typedef union MBAP {
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
    uint8_t _MBAP[7];
	cbModbusConnect cbConnect = NULL;
    public:
    void onConnect(cbModbusConnect cb);
    virtual IPAddress eventSource() {
		
	}
};
typedef struct TTransaction;
typedef uint16_t (*cbModbusSlave)(TTransaction* query, bool result);
typedef struct TTransaction {
	uint16_t	id;
	uint16_t	startreg;
	uint16_t	numregs;
	uint32_t	timestamp;
	TQuery*		next;
	cbModbusSlave cb;
}
class ModbusMasterIP : public ModbusCoreIP, public WiFiClient {
	private:
	TTransaction* _trans;
	uint8_t	   status;
	IPAddress	ip;
	uint32_t	queryStart;
	uint32_t	timeout;
	uint16_t	transactionId = 0;
	public:
	ModbusMasterIP() : WiFiClient() {
	}
	void connect(IPAddress address);
	void pushBits(uint16_t address, uint16_t numregs, modbusFunctionCode fn);
	void pullBits(uint16_t address, uint16_t numregs, modbusFunctionCode fn);
	void pushWords(uint16_t address, uint16_t numregs, modbusFunctionCode fn);
	void pullWords(uint16_t address, uint16_t numregs, modbusFunctionCode fn);
	bool send() {
		uint16_t i;
		//MBAP
		_MBAP[0] = 0;
		_MBAP[1] = 1;
		_MBAP[2] = 0;
		_MBAP[3] = 0;	
		_MBAP[4] = (_len+1) >> 8;     //_len+1 for last byte from MBAP
		_MBAP[5] = (_len+1) & 0x00FF;
		_MBAP[6] = 0xFF;
				
		size_t send_len = (uint16_t)_len + 7;
		uint8_t sbuf[send_len];
				
		for (i = 0; i < 7; i++)	    sbuf[i] = _MBAP[i];
		for (i = 0; i < _len; i++)	sbuf[i+7] = _frame[i];
		write(sbuf, send_len);
		//Serial.println(_frame[0]);
	}
	bool get() {
		uint8_t i;
		if (!connected()) return false;
		uint16_t raw_len = 0;
		raw_len = available();
		//Serial.println(raw_len);
		if (available() > sizeof(_MBAP)) {
			//for (i = 0; i < 7; i++)	_MBAP[i] = read(); //Get MBAP
			readBytes(_MBAP, sizeof(_MBAP));	//Get MBAP
			_len = _MBAP[4] << 8 | _MBAP[5];
			_len--; // Do not count with last byte from MBAP
			if (_MBAP[2] == 0 && _MBAP[3] == 0 && _len < MODBUSIP_MAXFRAME) {
				_frame = (uint8_t*) malloc(_len);
				//raw_len = raw_len - 7;
				//for (i = 0; i < _len; i++)
				//	_frame[i] = read(); //Get Modbus PDU
				if (readBytes(_frame, _len) == _len) {
					responcePDU(_frame);
					return true;
				}
			}
			flush();
		}
		return false;
	}
	void pushCoil() {
	}
	void pullCoil(uint16_t offset, uint16_t numregs = 1) {
		readSlave(offset, numregs, READ_COILS);
		send();
	}
	void pullCoils() {
		uint16_t offset;
		uint16_t numregs = 1;
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

class ModbusIP : public ModbusCoreIP, public WiFiServer {
    private:
    //uint8_t _MBAP[7];
	WiFiClient* client[MODBUSIP_MAX_CLIENTS];
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