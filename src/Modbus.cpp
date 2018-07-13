/*
    Modbus.cpp - Modbus Base Library Implementation
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2018 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "Modbus.h"

#ifdef MB_GLOBAL_REGS
std::vector<TRegister> _regs;
#endif

uint16_t cbDefault(TRegister* reg, uint16_t val) {
	return val;
}

TRegister* Modbus::searchRegister(TAddress address) {
    const TRegister tmp = {address, 0, cbDefault, cbDefault};
    std::vector<TRegister>::iterator it = std::find(_regs.begin(), _regs.end(), tmp);
    if (it != _regs.end()) return &*it;
    return nullptr;
}

bool Modbus::addReg(TAddress address, uint16_t value, uint16_t numregs) {
   #ifdef MB_MAX_REGS
    if (_regs.size() + numregs > MB_MAX_REGS) return false;
   #endif
    for (uint16_t i = 0; i < numregs; i++) {
        if (!searchRegister(address + i))
            _regs.push_back({address + i, value, cbDefault, cbDefault});
    }
    //std::sort(_regs.begin(), _regs.end());
    return true;
}

bool Modbus::Reg(TAddress address, uint16_t value) {
    TRegister* reg;
    reg = searchRegister(address); //search for the register address
    if (reg) { //if found then assign the register value to the new value.
        if (cbEnabled) {
            reg->value = reg->set(reg, value);
        } else {
            reg->value = value;
        }
        return true;
    } else 
        return false;
}

uint16_t Modbus::Reg(TAddress address) {
    TRegister* reg;
    reg = searchRegister(address);
    if(reg)
        if (cbEnabled) {
            return reg->get(reg, reg->value);
        } else {
            return reg->value;
        }
    else
        return 0;
}

bool Modbus::removeReg(TAddress address) {
    // Empty stub
}

void Modbus::slavePDU(uint8_t* frame) {
    FunctionCode fcode  = (FunctionCode)frame[0];
    uint16_t field1 = (uint16_t)frame[1] << 8 | (uint16_t)frame[2];
    uint16_t field2 = (uint16_t)frame[3] << 8 | (uint16_t)frame[4];
    uint16_t bytecount_calc;
    switch (fcode) {
        case FC_WRITE_REG:
            //field1 = reg, field2 = value
            if (!Hreg(field1, field2)) { //Check Address and execute (reg exists?)
                exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                break;
            }
            if (Hreg(field1) != field2) { //Check for failure
                exceptionResponse(fcode, EX_SLAVE_FAILURE);
                break;
            }
            _reply = REPLY_ECHO;
        break;

        case FC_READ_REGS:
            //field1 = startreg, field2 = numregs, header len = 3
            readWords(HREG(field1), field2, fcode);
        break;

        case FC_WRITE_REGS:
            //field1 = startreg, field2 = numregs, frame[5] = data lenght, header len = 6
            if (field2 < 0x0001 || field2 > 0x007B || frame[5] != 2 * field2) { //Check value
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                break;
            }
            for (int k = 0; k < field2; k++) { //Check Address (startreg...startreg + numregs)
                if (!searchRegister(HREG(field1) + k)) {
                    exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                    break;
                }
            }
            setMultipleWords(frame + 6, HREG(field1), field2);
            successResponce(HREG(field1), field2, fcode);
            _reply = REPLY_NORMAL;
        break;

        case FC_READ_COILS:
            //field1 = startreg, field2 = numregs
            readBits(COIL(field1), field2, fcode);
        break;

        case FC_READ_INPUT_STAT:
            //field1 = startreg, field2 = numregs
            readBits(ISTS(field1), field2, fcode);
        break;

        case FC_READ_INPUT_REGS:
            //field1 = startreg, field2 = numregs
            readWords(IREG(field1), field2, fcode);
        break;

        case FC_WRITE_COIL:
            //field1 = reg, field2 = status, header len = 3
            if (field2 != 0xFF00 && field2 != 0x0000) { //Check value (status)
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                break;
            }
            if (!Coil(field1, COIL_BOOL(field2))) { //Check Address and execute (reg exists?)
                exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                break;
            }
            if (Coil(field1) != COIL_BOOL(field2)) { //Check for failure
                exceptionResponse(fcode, EX_SLAVE_FAILURE);
                break;
            }
            _reply = REPLY_ECHO;
        break;

        case FC_WRITE_COILS:
            //field1 = startreg, field2 = numregs, frame[5] = bytecount, header len = 6
            bytecount_calc = field2 / 8;
            if (field2%8) bytecount_calc++;
            if (field2 < 0x0001 || field2 > 0x07B0 || frame[5] != bytecount_calc) { //Check registers range and data size maches
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                break;
            }
            for (int k = 0; k < field2; k++) { //Check Address (startreg...startreg + numregs)
                if (!searchRegister(COIL(field1) + k)) {
                    exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                    break;
                }
            }
            setMultipleBits(frame + 6, COIL(field1), field2);
            successResponce(COIL(field1), field2, fcode);
            _reply = REPLY_NORMAL;
        break;

        default:
            exceptionResponse(fcode, EX_ILLEGAL_FUNCTION);
    }
}

void Modbus::successResponce(TAddress startreg, uint16_t numoutputs, FunctionCode fn) {
    free(_frame);
	_len = 5;
    _frame = (uint8_t*) malloc(_len);
    _frame[0] = fn;
    _frame[1] = startreg.address >> 8;
    _frame[2] = startreg.address & 0x00FF;
    _frame[3] = numoutputs >> 8;
    _frame[4] = numoutputs & 0x00FF;
}

void Modbus::exceptionResponse(FunctionCode fn, ResultCode excode) {
    free(_frame);
    _len = 2;
    _frame = (uint8_t*) malloc(_len);
    _frame[0] = fn + 0x80;
    _frame[1] = excode;
    _reply = REPLY_NORMAL;
}

void Modbus::getMultipleBits(uint8_t* frame, TAddress startreg, uint16_t numregs) {
    uint8_t bitn = 0;
    uint16_t i = 0;
	while (numregs--) {
		if (BIT_BOOL(Reg(startreg)))
			bitSet(frame[i], bitn);
        else
			bitClear(frame[i], bitn);
		bitn++; //increment the bit index
		if (bitn == 8)  {
            i++;
            bitn = 0;
        }
		startreg++; //increment the register
	}
}

void Modbus::getMultipleWords(uint8_t* frame, TAddress startreg, uint16_t numregs) {
    uint16_t val;
    uint16_t i = 0;
	while(numregs--) {
        val = Reg(startreg + i);  //retrieve the value from the register bank for the current register
        frame[i * 2]  = val >> 8;       //write the high byte of the register value
        frame[i * 2 + 1] = val & 0xFF;  //write the low byte of the register value
        i++;
	}
}

void Modbus::readBits(TAddress startreg, uint16_t numregs, FunctionCode fn) {
    if (numregs < 0x0001 || numregs > 0x07D0) { //Check value (numregs)
        exceptionResponse(fn, EX_ILLEGAL_VALUE);
        return;
    }
    //Check Address
    //Check only startreg. Is this correct?
    //When I check all registers in range I got errors in ScadaBR
    //I think that ScadaBR request more than one in the single request
    //when you have more then one datapoint configured from same type.
    if (!searchRegister(startreg)) {
        exceptionResponse(fn, EX_ILLEGAL_ADDRESS);
        return;
    }
    free(_frame);
    //Determine the message length = function type, byte count and
	//for each group of 8 registers the message length increases by 1
	_len = 2 + numregs/8;
	if (numregs%8) _len++; //Add 1 to the message length for the partial byte.
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        exceptionResponse(fn, EX_SLAVE_FAILURE);
        return;
    }
    _frame[0] = fn;
    _frame[1] = _len - 2; //byte count (_len - function code and byte count)
	_frame[_len - 1] = 0;  //Clean last probably partial byte
    getMultipleBits(_frame+2, startreg, numregs);
    _reply = REPLY_NORMAL;
}

void Modbus::readWords(TAddress startreg, uint16_t numregs, FunctionCode fn) {
    //Check value (numregs)
    if (numregs < 0x0001 || numregs > 0x007D) {
        exceptionResponse(fn, EX_ILLEGAL_VALUE);
        return;
    }
    if (!searchRegister(startreg)) { //Check Address
        exceptionResponse(fn, EX_ILLEGAL_ADDRESS);
        return;
    }
    free(_frame);
	_len = 2 + numregs * 2; //calculate the query reply message length. 2 bytes per register + 2 bytes for header
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        exceptionResponse(fn, EX_SLAVE_FAILURE);
        return;
    }
    _frame[0] = fn;
    _frame[1] = _len - 2;   //byte count
    getMultipleWords(_frame + 2, startreg, numregs);
    _reply = REPLY_NORMAL;
}

void Modbus::setMultipleBits(uint8_t* frame, TAddress startreg, uint16_t numoutputs) {
    uint8_t bitn = 0;
    uint16_t i = 0;
	while (numoutputs--) {
        Reg(startreg, BIT_VAL(bitRead(frame[i], bitn)));
        bitn++;     //increment the bit index
        if (bitn == 8) {
            i++;
            bitn = 0;
        }
        startreg++; //increment the register
	}
}

void Modbus::setMultipleWords(uint8_t* frame, TAddress startreg, uint16_t numregs) {
    uint16_t val;
    uint16_t i = 0;
	while(numregs--) {
        val = (uint16_t)frame[i*2] << 8 | (uint16_t)frame[i*2 + 1];
        Reg(startreg + i, val);
        i++;
	}
}

bool Modbus::onGet(TAddress address, cbModbus cb, uint16_t numregs) {
	TRegister* reg;
	bool atLeastOne = false;
	while (numregs > 0) {
		reg = searchRegister(address);
		if (reg) {
			reg->get = cb;
			atLeastOne = true;
		}
		address++;
		numregs--;
	}
	return atLeastOne;
}
bool Modbus::onSet(TAddress address, cbModbus cb, uint16_t numregs) {
	TRegister* reg;
	bool atLeastOne = false;
	while (numregs > 0) {
		reg = searchRegister(address);
		if (reg) {
			reg->set = cb;
			atLeastOne = true;
		}
		address++;
		numregs--;
	}
	return atLeastOne;
}

bool Modbus::readSlave(TAddress address, uint16_t numregs, FunctionCode fn) {
	free(_frame);
	_len = 5;
	_frame = (uint8_t*) malloc(_len);
	_frame[0] = fn;
	_frame[1] = address.address >> 8;
	_frame[2] = address.address & 0x00FF;
	_frame[3] = numregs >> 8;
	_frame[4] = numregs & 0x00FF;
	return true;
}

bool Modbus::writeSlaveBits(TAddress startreg, uint16_t numregs, FunctionCode fn) {
	free(_frame);
	_len = 6 + numregs/8;
	if (numregs%8) _len++; //Add 1 to the message length for the partial byte.
    _frame = (uint8_t*) malloc(_len);
    if (_frame) {
	    _frame[0] = fn;
	    _frame[1] = startreg.address >> 8;
	    _frame[2] = startreg.address & 0x00FF;
	    _frame[3] = numregs >> 8;
	    _frame[4] = numregs & 0x00FF;
        _frame[5] = _len - 6;
        _frame[_len - 1] = 0;  //Clean last probably partial byte
        getMultipleBits(_frame + 6, startreg, numregs);
        _reply = REPLY_NORMAL;
        return true;
    }
    _reply = REPLY_OFF;
	return false;
}

bool Modbus::writeSlaveWords(TAddress startreg, uint16_t numregs, FunctionCode fn) {
	free(_frame);
	_len = 6 + 2 * numregs;
	_frame = (uint8_t*) malloc(_len);
    if (_frame) {
	    _frame[0] = fn;
	    _frame[1] = startreg.address >> 8;
	    _frame[2] = startreg.address & 0x00FF;
	    _frame[3] = numregs >> 8;
	    _frame[4] = numregs & 0x00FF;
        _frame[5] = _len - 6;
        getMultipleWords(_frame + 6, startreg, numregs);
        return true;
    }
    _reply = REPLY_OFF;
	return false;    
}

void Modbus::masterPDU(uint8_t* frame, uint8_t* sourceFrame) {
    uint8_t fcode  = frame[0];
    _reply = 0;
    if ((fcode & 0x80) != 0) {
	    _reply = _frame[1];
	    return;
    }
    uint16_t field1 = (uint16_t)sourceFrame[1] << 8 | (uint16_t)sourceFrame[2];
    uint16_t field2 = (uint16_t)sourceFrame[3] << 8 | (uint16_t)sourceFrame[4];
    uint8_t bytecount_calc;
    switch (fcode) {
        case FC_READ_REGS:
            //field1 = startreg, field2 = numregs, frame[1] = data lenght, header len = 2
            if (frame[1] != 2 * field2) { //Check if data size matches
                _reply = EX_DATA_MISMACH;
                break;
            }
            setMultipleWords(frame + 2, HREG(field1), field2);
        break;
        case FC_READ_COILS:
            //field1 = startreg, field2 = numregs, frame[1] = data length, header len = 2
            bytecount_calc = field2 / 8;
            if (field2%8) bytecount_calc++;
            if (frame[1] != bytecount_calc) { // check if data size matches
                _reply = EX_DATA_MISMACH;
                break;
            }
            setMultipleBits(frame + 2, COIL(field1), field2);
        break;
        case FC_READ_INPUT_STAT:
            //field1 = startreg, field2 = numregs, frame[1] = data length, header len = 2
            bytecount_calc = field2 / 8;
            if (field2%8) bytecount_calc++;
            if (frame[1] != bytecount_calc) { // check if data size matches
                _reply = EX_DATA_MISMACH;
                break;
            }
            setMultipleBits(frame + 2, ISTS(field1), field2);
        break;
        case FC_READ_INPUT_REGS:
            //field1 = startreg, field2 = status, frame[1] = data lenght, header len = 2
            if (frame[1] != 2 * field2) { //Check if data size matches
                _reply = EX_DATA_MISMACH;
                break;
            }
            setMultipleWords(frame + 2, IREG(field1), field2);
        break;
        case FC_WRITE_REG:
        break;
        case FC_WRITE_REGS:
        break;
        case FC_WRITE_COIL:
        break;
        case FC_WRITE_COILS:
        break;
        default:
		_reply = EX_GENERAL_FAILURE;
    }
}

void Modbus::cbEnable(bool state) {
    cbEnabled = state;
}