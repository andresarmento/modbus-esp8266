/*
    ModbusRTU.h - ModbusRTU Library
    Copyright (C) 2019 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once
#include <Modbus.h>
#include <HardwareSerial.h>
#include <SoftwareSerial.h>

#define MODBUSRTU_BROADCAST 0
#define MB_RESERVE 248
#define MB_SERIAL_BUFFER 128
#define MB_MAX_TIME 10
#define MODBUSRTU_TIMEOUT 1000
//#define MB_STATIC_FRAME 1

template <typename S>
// Possible declarations are
// ModbusRTU<HardwareSerial> mb(Serial);
// ModbusRTU<SoftwareSerial> mb(SoftSerial);
class ModbusRTU : public Modbus {
    protected:
        S* _port;
        int   _txPin;
		uint32_t _t15;
		unsigned int _t;	// frame delay in mS
		uint32_t t = 0;
		bool isMaster = false;
		uint8_t  _slaveId;
		uint32_t _timestamp = 0;
		cbTransaction _cb = nullptr;
		void* _data = nullptr;
		uint8_t* _sentFrame = nullptr;
		TAddress _sentReg = COIL(0);
		bool send(uint8_t slaveId, TAddress startreg, cbTransaction cb, void* data = nullptr, bool waitResponse = true);
		// Prepare and send ModbusIP frame. _frame buffer and _len should be filled with Modbus data
		// slaveId - slave id
		// startreg - first local register to save returned data to (miningless for write to slave operations)
		// cb - transaction callback function
		// data - if not null use buffer to save returned data instead of local registers
		bool cleanup(); 	// Free clients if not connected and remove timedout transactions and transaction with forced events
		uint16_t crc(uint8_t address, uint8_t* frame, uint8_t pdulen);
    public:
		/*
		Bits per Byte:	1 start bit
						8 data bits, least significant bit sent first
						1 bit for parity completion or stop bit
						1 stop bit
		*/
	
        bool begin(SoftwareSerial* port, uint32_t baud, int16_t txPin=-1);
	 #ifdef ESP8266
        bool begin(HardwareSerial* port, uint32_t baud, SerialConfig format, int16_t txPin=-1);
	 #else
	 	bool begin(HardwareSerial* port, uint32_t baud, uint16_t format, int16_t txPin=-1);
	 #endif
        void task();
		void master() { isMaster = true; };
		void slave(uint8_t slaveId) {_slaveId = slaveId;};
		uint8_t slave() { return _slaveId; }
        bool setSlaveId(uint8_t slaveId);
        uint8_t getSlaveId();
		uint16_t writeHreg(uint8_t slaveId, uint16_t offset, uint16_t value, cbTransaction cb = nullptr);
		uint16_t writeCoil(uint8_t slaveId, uint16_t offset, bool value, cbTransaction cb = nullptr);
		uint16_t readCoil(uint8_t slaveId, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr);
/*
	uint16_t writeCoil(uint8_t slaveId, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t writeHreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t readCoil(uint8_t slaveId, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t readIsts(uint8_t slaveId, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t readHreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t readIreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr);

	uint16_t pushCoil(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t pullCoil(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t pullIsts(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t pushHreg(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t pullHreg(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t pullIreg(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr);

	uint16_t pullHregToIreg(uint8_t slaveId, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t pullCoilToIsts(uint8_t slaveId, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t pushIstsToCoil(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr);
	uint16_t pushIregToHreg(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr);
*/
};

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

template <typename S>
uint16_t ModbusRTU<S>::crc(uint8_t address, uint8_t* frame, uint8_t pduLen) {
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

template <typename S>
bool ModbusRTU<S>::setSlaveId(uint8_t slaveId){
    _slaveId = slaveId;
    return true;
}

template <typename S>
uint8_t ModbusRTU<S>::getSlaveId() {
    return _slaveId;
}

#ifdef ESP8266
template <>
bool ModbusRTU<HardwareSerial>::begin(HardwareSerial* port, uint32_t baud, SerialConfig format, int16_t txPin) {
    port->begin(baud, format);
#else
template <>
bool ModbusRTU<HardwareSerial>::begin(HardwareSerial* port, uint32_t baud, uint16_t format, int16_t txPin) {
    port->begin(baud);
#endif
    port->begin(baud);
    _port = port;
    _txPin = txPin;
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

template <>
bool ModbusRTU<SoftwareSerial>::begin(SoftwareSerial* port, uint32_t baud, int16_t txPin) {
    port->begin(baud);
    _port = port;
    _txPin = txPin;
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

template <typename S>
bool ModbusRTU<S>::send(uint8_t slaveId, TAddress startreg, cbTransaction cb, void* data, bool waitResponse) {
    if (_slaveId) return false; // Break if waiting for previous request result
    uint16_t newCrc = crc(slaveId, _frame, _len);
    if (_txPin >= 0) {
        digitalWrite(_txPin, HIGH);
        delay(1);
    }
    _port->write(slaveId);  //Send slaveId
    for (uint8_t i = 0 ; i < _len ; i++) _port->write(_frame[i]); // Send PDU
    //Send CRC
    _port->write(newCrc >> 8);
    _port->write(newCrc & 0xFF);
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

template <typename S>
void ModbusRTU<S>::task() {
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
    u_int frameCrc = ((_frame[_len - 2] << 8) | _frame[_len - 1]); // Last two byts = crc
    _len = _len - 2;    // Decrease by CRC 2 bytes
    if (frameCrc != crc(address, _frame, _len)) {  // CRC Check
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
            if (cbEnabled && _cb) {
			    _cb((ResultCode)_reply, 0, nullptr);
		    }
            free(_sentFrame);
            _sentFrame = nullptr;
            _data = nullptr;
		    _slaveId = 0;
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
        uint16_t newCrc = crc(_slaveId, _frame, _len);
        _port->write(newCrc >> 8);
        _port->write(newCrc & 0xFF);
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

template <typename S>
bool ModbusRTU<S>::cleanup() {
	// Remove timeouted request and forced event
	if (_slaveId && (millis() - _timestamp > MODBUSRTU_TIMEOUT)) {
		_cb(Modbus::EX_TIMEOUT, 0, nullptr);
		free(_sentFrame);
        _sentFrame = nullptr;
        _data = nullptr;
		_slaveId = 0;
        return true;
	}
    return false;
}

template <typename S>
uint16_t ModbusRTU<S>::writeHreg(uint8_t slaveId, uint16_t offset, uint16_t value, cbTransaction cb) {
    readSlave(offset, value, FC_WRITE_REG);
	return send(slaveId, HREG(offset), cb, nullptr, cb);
}
template <typename S>
uint16_t ModbusRTU<S>::writeCoil(uint8_t slaveId, uint16_t offset, bool value, cbTransaction cb) {
	readSlave(offset, COIL_VAL(value), FC_WRITE_COIL);
	return send(slaveId, COIL(offset), cb, nullptr, cb);
}
template <typename S>
uint16_t ModbusRTU<S>::readCoil(uint8_t slaveId, uint16_t offset, bool* value, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x007B) return false;
	readSlave(offset, numregs, FC_READ_COILS);
	return send(slaveId, COIL(offset), cb, value);
}