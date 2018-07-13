/*
    ModbusSerial.cpp - Source for Modbus Serial Library
    Copyright (C) 2014 André Sarmento Barbosa
                  2017-2018 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "ModbusSerial.h"

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

ModbusSerial::ModbusSerial() {

}

bool ModbusSerial::setSlaveId(uint8_t slaveId){
    _slaveId = slaveId;
    return true;
}

uint8_t ModbusSerial::getSlaveId() {
    return _slaveId;
}

#ifdef MB_SOFTWARE_SERIAL
bool ModbusSerial::config(SoftwareSerial* port, uint32_t baud, int16_t txPin) {
    (*port).begin(baud);
#else
bool ModbusSerial::config(HardwareSerial* port, uint32_t baud, SerialConfig format, int16_t txPin) {
    (*port).begin(baud, format);
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

bool ModbusSerial::receive(uint8_t* frame) {
    //first byte of frame = address
    uint8_t address = frame[0];
    //Last two byts = crc
    u_int crc = ((frame[_len - 2] << 8) | frame[_len - 1]);

    //Slave Check
    if (address != 0xFF && address != getSlaveId()) {
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
    if (address == 0xFF) _reply = Modbus::REPLY_OFF;
    return true;
}

bool ModbusSerial::send(uint8_t* frame) {
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

bool ModbusSerial::sendPDU(uint8_t* pduframe) {
    if (_txPin >= 0) {
        digitalWrite(_txPin, HIGH);
        delay(1);
    }

    //Send slaveId
    (*_port).write(_slaveId);

    //Send PDU
    uint8_t i;
    for (i = 0 ; i < _len ; i++) {
        (*_port).write(pduframe[i]);
    }

    //Send CRC
    uint16_t crc = calcCrc(_slaveId, _frame, _len);
    (*_port).write(crc >> 8);
    (*_port).write(crc & 0xFF);

    (*_port).flush();
    delayMicroseconds(_t35);

    if (_txPin >= 0) {
        digitalWrite(_txPin, LOW);
    }
}

void ModbusSerial::task() {
    _len = 0;

    while ((*_port).available() > _len)	{
        _len = (*_port).available();
        delayMicroseconds(_t15);
    }

    if (_len == 0) return;

    uint8_t i;
    _frame = (uint8_t*) malloc(_len);
    for (i=0 ; i < _len ; i++) _frame[i] = (*_port).read();

    if (receive(_frame)) {
        if (_reply == Modbus::REPLY_NORMAL)
            sendPDU(_frame);
        else
        if (_reply == Modbus::REPLY_ECHO)
            send(_frame);
    }

    free(_frame);
    _len = 0;
}