/*
    ModbusSerial.cpp - Source for Modbus Serial Library
    Copyright (C) 2014 André Sarmento Barbosa
                  2017-2018 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "ModbusRTU.h"

/* calcCrc optimisation required:
1. Move inside class code
2. Merge CRC tables to one and move it to PROGMEM
*/
uint16_t calcCrc(uint8_t address, uint8_t* pduFrame, uint8_t pduLen) {
	uint8_t CRCHi = 0xFF, CRCLo = 0x0FF, Index;

    Index = CRCHi ^ address;
    CRCHi = CRCLo ^ _auchCRCHi[Index];
    CRCLo = _auchCRCLo[Index];

    while (pduLen--) {
        Index = CRCHi ^ *pduFrame++;
        CRCHi = CRCLo ^ _auchCRCHi[Index];
        CRCLo = _auchCRCLo[Index];
    }

    return (CRCHi << 8) | CRCLo;
}

bool ModbusRTUSlave::setSlaveId(uint8_t slaveId){
    _slaveId = slaveId;
    return true;
}

uint8_t ModbusRTUSlave::getSlaveId() {
    return _slaveId;
}

#ifdef MB_SOFTWARE_SERIAL
bool ModbusRTU::begin(SoftwareSerial* port, uint32_t baud, int16_t txPin) {
    (*port).begin(baud);
#else
#ifdef ESP8266
bool ModbusRTU::begin(HardwareSerial* port, uint32_t baud, SerialConfig format, int16_t txPin) {
    (*port).begin(baud, format);
#else
bool ModbusRTU::begin(HardwareSerial* port, uint32_t baud, uint16_t format, int16_t txPin) {
    (*port).begin(baud);
#endif
#endif
    _port = port;
    _txPin = txPin;

    delay(2000);    // ???

    if (txPin >= 0) {
        pinMode(txPin, OUTPUT);
        digitalWrite(txPin, LOW);
    }

    if (baud > 19200) {
        _t15 = 750;
        _t35 = 1750;
    } else {
        _t15 = 15000000/baud; // 1T * 1.5 = T1.5
        _t35 = 35000000/baud; // 1T * 3.5 = T3.5
    }

    return true;
}

bool ModbusRTU::receive(uint8_t* frame, uint8_t slaveId) {
    //first byte of frame = address
    uint8_t address = frame[0];
    //Last two byts = crc
    u_int crc = ((frame[_len - 2] << 8) | frame[_len - 1]);

    //Slave Check
    if (address != MB_BROADCAST && address != slaveId) {
		return false;
	}

    //CRC Check
    if (crc != calcCrc(_frame[0], _frame+1, _len-3)) {
		return false;
    }

    //PDU starts after first uint8_t
    //framesize PDU = framesize - address(1) - crc(2)
    slavePDU(frame+1);
    //No reply to Broadcasts
    if (address == MB_BROADCAST) _reply = Modbus::REPLY_OFF;
    return true;
}

bool ModbusRTU::send(uint8_t* frame) {
    uint8_t i;

    if (_txPin >= 0) {
        digitalWrite(_txPin, HIGH);
        delay(1);
    }

    for (i = 0 ; i < _len ; i++) {
        (*_port).write(frame[i]);
    }

    (*_port).flush();
    delayMicroseconds(_t35);

    if (_txPin >= 0) {
        digitalWrite(_txPin, LOW);
    }
}

bool ModbusRTU::sendPDU(uint8_t* pduframe, uint8_t slaveId) {
    if (_txPin >= 0) {
        digitalWrite(_txPin, HIGH);
        delay(1);
    }

    //Send slaveId
    (*_port).write(slaveId);

    //Send PDU
    uint8_t i;
    for (i = 0 ; i < _len ; i++) {
        (*_port).write(pduframe[i]);
    }

    //Send CRC
    uint16_t crc = calcCrc(slaveId, _frame, _len);
    (*_port).write(crc >> 8);
    (*_port).write(crc & 0xFF);

    (*_port).flush();
    delayMicroseconds(_t35);

    if (_txPin >= 0) {
        digitalWrite(_txPin, LOW);
    }
}
/*
task() function is totaly wrong:
1. Serial bufer overfull may happen. 
2. Up to ~0.3 sec execution time at 9600. Needs to add some logic to be async at low rates (9600 and may be 19200).
3. No frame size (256 is max) checking
count = MB_MAX_FRAME;
if (.available()) { while(.available() > 0 && count > MB_SERIAL_BUFFER) { read(); count--;}; return(); }
if (_len > 0) {
    process();
}

*/
void ModbusRTUSlave::task() {
    _len = 0;

    while ((*_port).available() > _len)	{
        _len = (*_port).available();
        delayMicroseconds(_t15);
    }

    if (_len == 0) return;
    Serial.println(_len);
    uint8_t i;
    //_frame = (uint8_t*) malloc(_len);
    _frame = (uint8_t*) malloc(MB_MAX_FRAME);
    for (i=0 ; i < _len ; i++) _frame[i] = (*_port).read();

    if (receive(_frame, _slaveId)) {
        if (_reply == Modbus::REPLY_NORMAL)
            sendPDU(_frame, _slaveId);
        else
        if (_reply == Modbus::REPLY_ECHO)
            send(_frame);
    }

    free(_frame);
    _len = 0;
}