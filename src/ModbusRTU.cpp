/*
    Modbus Library for Arduino
    ModbusRTU implementation
    Copyright (C) 2019-2022 Alexander Emelianov (a.m.emelianov@gmail.com)
	https://github.com/emelianov/modbus-esp8266
	This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/
#include "ModbusRTU.h"

// Table of CRC values
static const uint16_t _auchCRC[] PROGMEM = {
	0x0000, 0xC1C0, 0x81C1, 0x4001, 0x01C3, 0xC003, 0x8002, 0x41C2, 0x01C6, 0xC006, 0x8007, 0x41C7, 0x0005, 0xC1C5, 0x81C4,
	0x4004, 0x01CC, 0xC00C, 0x800D, 0x41CD, 0x000F, 0xC1CF, 0x81CE, 0x400E, 0x000A, 0xC1CA, 0x81CB, 0x400B, 0x01C9, 0xC009,
	0x8008, 0x41C8, 0x01D8, 0xC018, 0x8019, 0x41D9, 0x001B, 0xC1DB, 0x81DA, 0x401A, 0x001E, 0xC1DE, 0x81DF, 0x401F, 0x01DD,
	0xC01D, 0x801C, 0x41DC, 0x0014, 0xC1D4, 0x81D5, 0x4015, 0x01D7, 0xC017, 0x8016, 0x41D6, 0x01D2, 0xC012, 0x8013, 0x41D3,
	0x0011, 0xC1D1, 0x81D0, 0x4010, 0x01F0, 0xC030, 0x8031, 0x41F1, 0x0033, 0xC1F3, 0x81F2, 0x4032, 0x0036, 0xC1F6, 0x81F7,
	0x4037, 0x01F5, 0xC035, 0x8034, 0x41F4, 0x003C, 0xC1FC, 0x81FD, 0x403D, 0x01FF, 0xC03F, 0x803E, 0x41FE, 0x01FA, 0xC03A,
	0x803B, 0x41FB, 0x0039, 0xC1F9, 0x81F8, 0x4038, 0x0028, 0xC1E8, 0x81E9, 0x4029, 0x01EB, 0xC02B, 0x802A, 0x41EA, 0x01EE,
	0xC02E, 0x802F, 0x41EF, 0x002D, 0xC1ED, 0x81EC, 0x402C, 0x01E4, 0xC024, 0x8025, 0x41E5, 0x0027, 0xC1E7, 0x81E6, 0x4026,
	0x0022, 0xC1E2, 0x81E3, 0x4023, 0x01E1, 0xC021, 0x8020, 0x41E0, 0x01A0, 0xC060, 0x8061, 0x41A1, 0x0063, 0xC1A3, 0x81A2,
	0x4062, 0x0066, 0xC1A6, 0x81A7, 0x4067, 0x01A5, 0xC065, 0x8064, 0x41A4, 0x006C, 0xC1AC, 0x81AD, 0x406D, 0x01AF, 0xC06F,
	0x806E, 0x41AE, 0x01AA, 0xC06A, 0x806B, 0x41AB, 0x0069, 0xC1A9, 0x81A8, 0x4068, 0x0078, 0xC1B8, 0x81B9, 0x4079, 0x01BB,
	0xC07B, 0x807A, 0x41BA, 0x01BE, 0xC07E, 0x807F, 0x41BF, 0x007D, 0xC1BD, 0x81BC, 0x407C, 0x01B4, 0xC074, 0x8075, 0x41B5,
	0x0077, 0xC1B7, 0x81B6, 0x4076, 0x0072, 0xC1B2, 0x81B3, 0x4073, 0x01B1, 0xC071, 0x8070, 0x41B0, 0x0050, 0xC190, 0x8191,
	0x4051, 0x0193, 0xC053, 0x8052, 0x4192, 0x0196, 0xC056, 0x8057, 0x4197, 0x0055, 0xC195, 0x8194, 0x4054, 0x019C, 0xC05C,
	0x805D, 0x419D, 0x005F, 0xC19F, 0x819E, 0x405E, 0x005A, 0xC19A, 0x819B, 0x405B, 0x0199, 0xC059, 0x8058, 0x4198, 0x0188,
	0xC048, 0x8049, 0x4189, 0x004B, 0xC18B, 0x818A, 0x404A, 0x004E, 0xC18E, 0x818F, 0x404F, 0x018D, 0xC04D, 0x804C, 0x418C,
	0x0044, 0xC184, 0x8185, 0x4045, 0x0187, 0xC047, 0x8046, 0x4186, 0x0182, 0xC042, 0x8043, 0x4183, 0x0041, 0xC181, 0x8180,
	0x4040, 0x0000
};

uint16_t ModbusRTUTemplate::crc16(uint8_t address, uint8_t* frame, uint8_t pduLen) {
	uint8_t i = 0xFF ^ address;
	uint16_t val = pgm_read_word(_auchCRC + i);
    uint8_t CRCHi = 0xFF ^ highByte(val);	// Hi
    uint8_t CRCLo = lowByte(val);	//Low
    while (pduLen--) {
        i = CRCHi ^ *frame++;
        val = pgm_read_word(_auchCRC + i);
        CRCHi = CRCLo ^ highByte(val);	// Hi
        CRCLo = lowByte(val);	//Low
    }
    return (CRCHi << 8) | CRCLo;
}
/*
uint16_t ModbusRTUTemplate::crc16_alt(uint8_t address, uint8_t* frame, uint8_t pduLen) {
  uint16_t temp, temp2, flag;
  temp = 0xFFFF ^ address;
  for (uint8_t i = 0; i < pduLen; i++)
  {
    temp = temp ^ frame[i];
    for (uint8_t j = 1; j <= 8; j++)
    {
      flag = temp & 0x0001;
      temp >>= 1;
      if (flag)
        temp ^= 0xA001;
    }
  }
  // Reverse byte order. 
  temp2 = temp >> 8;
  temp = (temp << 8) | temp2;
  temp &= 0xFFFF;
  return temp;
}
*/

uint32_t ModbusRTUTemplate::charSendTime(uint32_t baud, uint8_t char_bits) {
	return (uint32_t)char_bits * 1000000UL / baud;
}

uint32_t ModbusRTUTemplate::calculateMinimumInterFrameTime(uint32_t baud, uint8_t char_bits) {
	// baud = baudrate of the serial port
	// char_bits = size of 1 modbus character (defined a 11 bits in modbus specificacion)
	// Returns: The minimum time between frames (defined as 3.5 characters time in modbus specification)
	
	// According to standard, the Modbus frame is always 11 bits long:
	// 1 start + 8 data + 1 parity + 1 stop
	// 1 start + 8 data + 2 stops
	// And the minimum time between frames is defined as 3.5 characters time in modbus specification.
	// This means the time between frames (in microseconds) should be calculated as follows:
	// _t = 3.5 x 11 x 1000000 / baudrate = 38500000 / baudrate

	// Eg: For 9600 baudrate _t = 38500000 / 9600 = 4010 us
	// For baudrates grater than 19200 the _t should be fixed at 1750 us.
	
	// If the used modbus frame length is 10 bits (out of standard - 1 start + 8 data + 1 stop), then 
	// it can be set using char_bits = 10.
    
	if (baud > 19200) {
        return 1750UL;
    } else {
		return 3.5 * charSendTime(baud, char_bits);
    }
}

// Kept for backward compatibility
void ModbusRTUTemplate::setBaudrate(uint32_t baud) {
    setInterFrameTime(calculateMinimumInterFrameTime(baud));
}

void ModbusRTUTemplate::setInterFrameTime(uint32_t t_us) {
	// This function sets the inter frame time. This time is the time that task() waits before considering that the frame being transmitted on the RS485 bus has finished.
	// If the interframe calculated by calculateMinimumInterFrameTime() is not enough, you can set the interframe time manually with this function. 
	// The time must be set in micro seconds. 
	// This is useful when you are receiving data as a slave and you notice that the slave is dividing a frame in two or more pieces (and obviously the CRC is failing on all pieces).
	// This is because it is detecting an interframe time inbetween bytes of the frame and thus it interprets one single frame as two or more frames.
	// In that case it is useful to be able to set a more "permissive" interframe time.
    _t = t_us;
}

bool ModbusRTUTemplate::begin(Stream* port, int16_t txEnablePin, bool txEnableDirect) {
    _port = port;
    _t = 1750UL;
#if defined(MODBUSRTU_FLUSH_DELAY)
	_t1 = charSendTime(0);
#endif
    if (txEnablePin >= 0) {
	    _txEnablePin = txEnablePin;
		_direct = txEnableDirect;
        pinMode(_txEnablePin, OUTPUT);
        digitalWrite(_txEnablePin, _direct?LOW:HIGH);
    }
    return true;
}

bool ModbusRTUTemplate::rawSend(uint8_t slaveId, uint8_t* frame, uint8_t len) {
    uint16_t newCrc = crc16(slaveId, frame, len);
#if defined(MODBUSRTU_DEBUG)
	for (uint8_t i=0 ; i < _len ; i++) {
		Serial.print(_frame[i], HEX);
		Serial.print(" ");
	}
	Serial.println();
#endif
#if defined(MODBUSRTU_REDE)
	if (_txEnablePin >= 0 || _rxPin >= 0) {
    	if (_txEnablePin >= 0)
        	digitalWrite(_txEnablePin, _direct?HIGH:LOW);
		if (_rxPin >= 0)
        	digitalWrite(_rxPin, _direct?HIGH:LOW);
#if !defined(ESP32)
        delayMicroseconds(MODBUSRTU_REDE_SWITCH_US);
#endif
	}
#else
    if (_txEnablePin >= 0) {
        digitalWrite(_txEnablePin, _direct?HIGH:LOW);
#if !defined(ESP32)
        delayMicroseconds(MODBUSRTU_REDE_SWITCH_US);
#endif
	}
#endif
#if defined(ESP32)
	vTaskDelay(0);
#endif
    _port->write(slaveId);  	//Send slaveId
    _port->write(frame, len); 	// Send PDU
    _port->write(newCrc >> 8);	//Send CRC
    _port->write(newCrc & 0xFF);//Send CRC
    _port->flush();
#if defined(MODBUSRTU_REDE)
	if (_txEnablePin >= 0 || _rxPin >= 0) {
#if defined(MODBUSRTU_FLUSH_DELAY)
		delayMicroseconds(_t1 * MODBUSRTU_FLUSH_DELAY);
#endif
    	if (_txEnablePin >= 0)
        	digitalWrite(_txEnablePin, _direct?LOW:HIGH);
		if (_rxPin >= 0)
        	digitalWrite(_rxPin, _direct?LOW:HIGH);
	}
#else
    if (_txEnablePin >= 0) {
#if defined(MODBUSRTU_FLUSH_DELAY)
		delayMicroseconds(_t1 * MODBUSRTU_FLUSH_DELAY);
#endif
        digitalWrite(_txEnablePin, _direct?LOW:HIGH);
	}
#endif
    return true;
}

uint16_t ModbusRTUTemplate::send(uint8_t slaveId, TAddress startreg, cbTransaction cb, uint8_t unit, uint8_t* data, bool waitResponse) {
    bool result = false;
	if ((!isMaster || !_slaveId) && _len && _frame) { // Check if waiting for previous request result and _frame filled
	//if (_len && _frame) { // Check if waiting for previous request result and _frame filled
		rawSend(slaveId, _frame, _len);
		if (waitResponse && slaveId) {
        	_slaveId = slaveId;
			_timestamp = micros();
			_cb = cb;
			_data = data;
			_sentFrame = _frame;
			_sentReg = startreg;
			_frame = nullptr;
		}
		result = true;
	}
	free(_frame);
	_frame = nullptr;
	_len = 0;
	return result;
}

void ModbusRTUTemplate::task() {
#if defined(ESP32)
	vTaskDelay(0);
#endif
    if (_port->available() > _len) {
        _len = _port->available();
        t = micros();
    }
	if (_len == 0) {
		if (isMaster) cleanup();
		return;
	}
	if (isMaster) {
		if (micros() - t < _t) {
			return;
		}
	}
	else {	// For slave wait for whole message to come (unless MODBUSRTU_MAX_READMS reached)
		uint32_t taskStart = micros();
    	while (micros() - t < _t) { // Wait data whitespace
    		if (_port->available() > _len) {
        		_len = _port->available();
        		t = micros();
			}
			if (micros() - taskStart > MODBUSRTU_MAX_READ_US) { // Prevent from task() executed too long
				return;
			}
		}
	}

	bool valid_frame = true;
    address = _port->read(); //first byte of frame = address
    _len--; // Decrease by slaveId byte
    if (isMaster && _slaveId == 0) {    // Check if slaveId is set
		valid_frame = false;
    }
    if (address != MODBUSRTU_BROADCAST && address != _slaveId) {     // SlaveId Check
		valid_frame = false;
    }
	if (!valid_frame && !_cbRaw) {
        for (uint8_t i=0 ; i < _len ; i++) _port->read();   // Skip packet if SlaveId doesn't mach
        _len = 0;
		if (isMaster) cleanup();
        return;
	}

	free(_frame);	//Just in case
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {  // Fail to allocate buffer
      for (uint8_t i=0 ; i < _len ; i++) _port->read(); // Skip packet if can't allocate buffer
      _len = 0;
	  if (isMaster) cleanup();
      return;
    }
    for (uint8_t i=0 ; i < _len ; i++) {
		_frame[i] = _port->read();   // read data + crc
		#if defined(MODBUSRTU_DEBUG)
		Serial.print(_frame[i], HEX);
		Serial.print(" ");
		#endif
	}
	#if defined(MODBUSRTU_DEBUG)
	Serial.println();
	#endif
	//_port->readBytes(_frame, _len);
    uint16_t frameCrc = ((_frame[_len - 2] << 8) | _frame[_len - 1]); // Last two byts = crc
    _len = _len - 2;    // Decrease by CRC 2 bytes
    if (frameCrc != crc16(address, _frame, _len)) {  // CRC Check
		goto cleanup;
    }
	_reply = EX_PASSTHROUGH;
	if (_cbRaw) {
		frame_arg_t header_data = { address, !isMaster };
		_reply = _cbRaw(_frame, _len, (void*)&header_data);
	}
	if (!valid_frame && _reply != EX_FORCE_PROCESS) {
		goto cleanup;
	}
    if (isMaster) {
        if ((_frame[0] & 0x7F) == _sentFrame[0]) { // Check if function code the same as requested
			// Procass incoming frame as master
			if (_reply == EX_PASSTHROUGH || _reply == EX_FORCE_PROCESS)
				masterPDU(_frame, _sentFrame, _sentReg, _data);
            if (_cb) {
			    _cb((ResultCode)_reply, 0, nullptr);
				_cb = nullptr;
		    }
            free(_sentFrame);
            _sentFrame = nullptr;
            _data = nullptr;
		    _slaveId = 0;
		}
        _reply = Modbus::REPLY_OFF;    // No reply if master
    } else {
		if (_reply == EX_PASSTHROUGH || _reply == EX_FORCE_PROCESS) {
        	slavePDU(_frame);
        	if (address == MODBUSRTU_BROADCAST)
				_reply = Modbus::REPLY_OFF;    // No reply for Broadcasts
    		if (_reply != Modbus::REPLY_OFF)
				rawSend(address, _frame, _len);
		}
    }
    // Cleanup
cleanup:
    free(_frame);
    _frame = nullptr;
    _len = 0;
	if (isMaster) cleanup();
}

bool ModbusRTUTemplate::cleanup() {
	// Remove timeouted request and forced event
	if (_slaveId && (micros() - _timestamp > MODBUSRTU_TIMEOUT_US)) {
		if (_cb) {
			_cb(Modbus::EX_TIMEOUT, 0, nullptr);
			_cb = nullptr;
		}
		free(_sentFrame);
        _sentFrame = nullptr;
        _data = nullptr;
		_slaveId = 0;
        return true;
	}
    return false;
}