/*
    ModbusSerial.cpp - Source for Modbus Serial Library
    Copyright (C) 2014 André Sarmento Barbosa
                  2017-2018 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include <ModbusRTU.h>

/* calcCrc optimisation required:
1. Move inside class code
2. Merge CRC tables to one and move it to PROGMEM
3. in task() probably don't read crc to buffer?
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
    port->begin(baud);
#else
#ifdef ESP8266
bool ModbusRTU::begin(HardwareSerial* port, uint32_t baud, SerialConfig format, int16_t txPin) {
    port->begin(baud, format);
#else
bool ModbusRTU::begin(HardwareSerial* port, uint32_t baud, uint16_t format, int16_t txPin) {
    port->begin(baud);
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
        _t = 2;
    } else {
        _t15 = 15000000/baud; // 1T * 1.5 = T1.5
        _t35 = 35000000/baud; // 1T * 3.5 = T3.5
        _t = (_t35 / 1000) + 1;
        Serial.print(_t);
    }

    return true;
}

void ModbusRTUSlave::task() {
    if (_port->available() > _len)	{
        _len = _port->available();
        t = millis();
        return;
    }
    if (_len == 0) return;  // No data
    if (millis() - t < _t) return;  // Wait data whitespace

    uint8_t address = _port->read(); //first byte of frame = address
    _len--; // Decrease by slaveId byte
    if (address != MB_BROADCAST && address != _slaveId) {     // SlaveId Check
        for (uint8_t i=0 ; i < _len ; i++) _port->read();   // Skip packet if SlaveId doesn't mach
        _len = 0;
        return;
    }

    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {  // Fail to allocate buffer
      for (uint8_t i=0 ; i < _len ; i++) _port->read(); // Skip packet if can't allocate buffer
      _len = 0;
      return;
    }
    for (uint8_t i=0 ; i < _len ; i++) _frame[i] = _port->read();   // read data + crc
    u_int crc = ((_frame[_len - 2] << 8) | _frame[_len - 1]); // Last two byts = crc
    _len = _len - 2;    // Decrease by CRC 2 bytes
    if (crc != calcCrc(address, _frame, _len)) {  // CRC Check
        _len = 0;   // Cleanup if wrong crc
        free(_frame);
        _frame = nullptr;
        return;
    }

    slavePDU(_frame);

    if (address == MB_BROADCAST) _reply = Modbus::REPLY_OFF;    //No reply to Broadcasts
    if (_reply != Modbus::REPLY_OFF) {
        if (_txPin >= 0) {
            digitalWrite(_txPin, HIGH);
            delay(1);
        }
        _port->write(_slaveId);  //Send slaveId
        for (uint8_t i = 0 ; i < _len ; i++) _port->write(_frame[i]); // Send PDU
        //Send CRC
        uint16_t crc = calcCrc(_slaveId, _frame, _len);
        _port->write(crc >> 8);
        _port->write(crc & 0xFF);
        _port->flush();
        delay(_t); //delayMicroseconds(_t35);
        if (_txPin >= 0) {
            digitalWrite(_txPin, LOW);
        }
    }
    // Cleanup
    _len = 0;
    free(_frame);
    _frame = nullptr;
}