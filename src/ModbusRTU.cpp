/*
    ModbusRTU.cpp - Source for ModbusRTU Library
    Copyright (C) 2019 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include <ModbusRTU.h>

/* calcCrc optimisation required:
1. Move inside class code
2. in task() probably don't read crc to buffer?
*/

uint16_t calcCrc(uint8_t address, uint8_t* pduFrame, uint8_t pduLen) {
	uint8_t Index = 0xFF ^ address;
	uint16_t val = pgm_read_word(_auchCRC + Index);
    uint8_t CRCHi = 0xFF ^ highByte(val);	// Hi
    uint8_t CRCLo = lowByte(val);	//Low
    while (pduLen--) {
        Index = CRCHi ^ *pduFrame++;
        val = pgm_read_word(_auchCRC + Index);
        CRCHi = CRCLo ^ highByte(val);	// Hi
        CRCLo = lowByte(val);	//Low
    }

    return (CRCHi << 8) | CRCLo;
}

bool ModbusRTU::setSlaveId(uint8_t slaveId){
    _slaveId = slaveId;
    return true;
}

uint8_t ModbusRTU::getSlaveId() {
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
        _t = 2;
    } else {
        _t = (35000/baud) + 1;
    }
    return true;
}

bool ModbusRTU::send(uint8_t slaveId, TAddress startreg, cbTransaction cb, void* data, bool waitResponse, uint8_t unit) {
    if (_slaveId) return false; // Break if waiting for previous request result
    uint16_t crc = calcCrc(slaveId, _frame, _len);
    if (_txPin >= 0) {
        digitalWrite(_txPin, HIGH);
        delay(1);
    }
    _port->write(slaveId);  //Send slaveId
    for (uint8_t i = 0 ; i < _len ; i++) _port->write(_frame[i]); // Send PDU
    //Send CRC
    _port->write(crc >> 8);
    _port->write(crc & 0xFF);
    _port->flush();
    delay(_t);
    if (_txPin >= 0) {
        digitalWrite(_txPin, LOW);
    }
	if (waitResponse) {
        _slaveId = slaveId;
		_timestamp = millis();
		_cb = cb;
		_data = data;
		_sentFrame = _frame;
		_sentReg = startreg;
		_frame = nullptr;
		_len = 0;
	}
	return true;
}

void ModbusRTU::task() {
    if (_port->available() > _len)	{
        _len = _port->available();
        t = millis();
        return;
    }
    if (_len == 0) {    // No data
        if (isMaster) cleanup();
        return;  
    }
    if (millis() - t < _t) return;  // Wait data whitespace

    uint8_t address = _port->read(); //first byte of frame = address
    _len--; // Decrease by slaveId byte
    if (isMaster && _slaveId == 0) {    // Check is slaveId is set
        for (uint8_t i=0 ; i < _len ; i++) _port->read();   // Skip packet if is not expected
        _len = 0;
        return;
    }
    if (address != MODBUSRTU_BROADCAST && address != _slaveId) {     // SlaveId Check
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
    if (isMaster) {
        _reply = EX_SUCCESS;
        if ((_frame[0] & 0x7F) == _sentFrame[0]) { // Check if function code the same as requested
			// Procass incoming frame as master
			masterPDU(_frame, _sentFrame, _sentReg, _data);
		} else {
			_reply = EX_UNEXPECTED_RESPONSE;
		}
		if (cbEnabled && _cb) {
			_cb((ResultCode)_reply, 0, nullptr);
		}
        _reply = Modbus::REPLY_OFF;    // No reply if master
    } else {
        slavePDU(_frame);
        if (address == MODBUSRTU_BROADCAST) _reply = Modbus::REPLY_OFF;    // No reply for Broadcasts
    }

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

bool ModbusRTU::cleanup() {
	// Remove timedouted request and forced event
	if (millis() - _timestamp > MODBUSRTU_TIMEOUT) {
		_cb(Modbus::EX_TIMEOUT, 0, nullptr);
		free(_sentFrame);
        _sentFrame = nullptr;
        _data = nullptr;
		_slaveId = 0;
        return true;
	}
    return false;
}