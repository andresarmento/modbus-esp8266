/*
    Modbus.h - Header for Modbus Base Library
    Copyright (C) 2014 André Sarmento Barbosa
                  2017 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "Modbus.h"

uint16_t cbDefault(TRegister* reg, uint16_t val) {
	return val;
}

TRegister* Modbus::searchRegister(uint16_t address) {
    TRegister *reg = _regs_head;

    //scan through the linked list until the end of the list or the register is found.
    //return the pointer.
    while (reg) {
        if (reg->address == address) return reg;
        reg = reg->next;
    }
    return NULL;
}

bool Modbus::addReg(uint16_t address, uint16_t value, uint16_t numregs) {
    TRegister *newreg;
    TRegister *root = NULL;
	while (numregs > 0) {
		newreg = (TRegister*) malloc(sizeof(TRegister));
		if (!newreg) {
			newreg = root;
			while (newreg) {
				root = newreg->next;
				free(newreg);
			}
			return false;
		}
		newreg->address = address;
		newreg->value	= value;
		newreg->get	= cbDefault;
		newreg->set	= cbDefault;
		newreg->next	= root;
		root = newreg;
		numregs--;
		address++;
	}
        _regs_head = root;
    return true;
}

bool Modbus::Reg(uint16_t address, uint16_t value) {
    TRegister *reg;
    //search for the register address
    reg = this->searchRegister(address);
    //if found then assign the register value to the new value.
    if (reg) {
        reg->value = reg->set(reg, value);
        return true;
    } else
        return false;
}

uint16_t Modbus::Reg(uint16_t address) {
    TRegister *reg;
    reg = this->searchRegister(address);
    if(reg)
        return reg->get(reg, reg->value);
    else
        return 0;
}

bool Modbus::addHreg(uint16_t offset, uint16_t value, uint16_t numregs) {
    return this->addReg(HREG(offset), value, numregs);
}

bool Modbus::Hreg(uint16_t offset, uint16_t value) {
    return Reg(HREG(offset), value);
}

uint16_t Modbus::Hreg(uint16_t offset) {
    return Reg(HREG(offset));
}

#ifndef USE_HOLDING_REGISTERS_ONLY
    bool Modbus::addCoil(uint16_t offset, bool value, uint16_t numregs) {
        return this->addReg(COIL(offset), COIL_VAL(value), numregs);
    }

    bool Modbus::addIsts(uint16_t offset, bool value, uint16_t numregs) {
        return this->addReg(ISTS(offset), ISTS_VAL(value), numregs);
    }

    bool Modbus::addIreg(uint16_t offset, uint16_t value, uint16_t numregs) {
        return this->addReg(IREG(offset), value, numregs);
    }

    bool Modbus::Coil(uint16_t offset, bool value) {
        return Reg(COIL(offset), COIL_VAL(value));
    }

    bool Modbus::Ists(uint16_t offset, bool value) {
        return Reg(ISTS(offset), ISTS_VAL(value));
    }

    bool Modbus::Ireg(uint16_t offset, uint16_t value) {
        return Reg(IREG(offset), value);
    }

    bool Modbus::Coil(uint16_t offset) {
        return COIL_BOOL(Reg(COIL(offset)));
    }

    bool Modbus::Ists(uint16_t offset) {
        return ISTS_BOOL(Reg(ISTS(offset)));
    }

    uint16_t Modbus::Ireg(uint16_t offset) {
        return Reg(IREG(offset));
    }
#endif


void Modbus::receivePDU(uint8_t* frame) {
    uint8_t fcode  = frame[0];
    uint16_t field1 = (word)frame[1] << 8 | (word)frame[2];
    uint16_t field2 = (word)frame[3] << 8 | (word)frame[4];

    switch (fcode) {

        case MB_FC_WRITE_REG:
            //field1 = reg, field2 = value
            this->writeSingleRegister(field1, field2);
        break;

        case MB_FC_READ_REGS:
            //field1 = startreg, field2 = numregs
            this->readRegisters(field1, field2);
        break;

        case MB_FC_WRITE_REGS:
            //field1 = startreg, field2 = status
            this->writeMultipleRegisters(frame,field1, field2, frame[5]);
        break;

        #ifndef USE_HOLDING_REGISTERS_ONLY
        case MB_FC_READ_COILS:
            //field1 = startreg, field2 = numregs
            this->readCoils(field1, field2);
        break;

        case MB_FC_READ_INPUT_STAT:
            //field1 = startreg, field2 = numregs
            this->readInputStatus(field1, field2);
        break;

        case MB_FC_READ_INPUT_REGS:
            //field1 = startreg, field2 = numregs
            this->readInputRegisters(field1, field2);
        break;

        case MB_FC_WRITE_COIL:
            //field1 = reg, field2 = status
            this->writeSingleCoil(field1, field2);
        break;

        case MB_FC_WRITE_COILS:
            //field1 = startreg, field2 = numoutputs
            this->writeMultipleCoils(frame,field1, field2, frame[5]);
        break;

        #endif
        default:
            this->exceptionResponse(fcode, MB_EX_ILLEGAL_FUNCTION);
    }
}

void Modbus::exceptionResponse(uint8_t fcode, uint8_t excode) {
    //Clean frame buffer
    free(_frame);
    _len = 2;
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
	// Don't send reply if can't build frame
	_reply = MB_REPLY_OFF;
	return;
    }
    _frame[0] = fcode + 0x80;
    _frame[1] = excode;

    _reply = MB_REPLY_NORMAL;
}

void Modbus::readRegisters(uint16_t startreg, uint16_t numregs) {
    //Check value (numregs)
    if (numregs < 0x0001 || numregs > 0x007D) {
        this->exceptionResponse(MB_FC_READ_REGS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address
    //*** See comments on readCoils method.
    if (!this->searchRegister(HREG(startreg))) {
        this->exceptionResponse(MB_FC_READ_REGS, MB_EX_ILLEGAL_ADDRESS);
        return;
    }


    //Clean frame buffer
    free(_frame);
	_len = 0;

	//calculate the query reply message length
	//for each register queried add 2 bytes
	_len = 2 + numregs * 2;

    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_READ_REGS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_READ_REGS;
    _frame[1] = _len - 2;   //byte count

    uint16_t val;
    uint16_t i = 0;
	while(numregs--) {
        //retrieve the value from the register bank for the current register
        val = this->Hreg(startreg + i);
        //write the high byte of the register value
        _frame[2 + i * 2]  = val >> 8;
        //write the low byte of the register value
        _frame[3 + i * 2] = val & 0xFF;
        i++;
	}

    _reply = MB_REPLY_NORMAL;
}

void Modbus::writeSingleRegister(uint16_t reg, uint16_t value) {
    //No necessary verify illegal value (EX_ILLEGAL_VALUE) - because using uint16_t (0x0000 - 0x0FFFF)
    //Check Address and execute (reg exists?)
    if (!this->Hreg(reg, value)) {
        this->exceptionResponse(MB_FC_WRITE_REG, MB_EX_ILLEGAL_ADDRESS);
        return;
    }

    //Check for failure
    if (this->Hreg(reg) != value) {
        this->exceptionResponse(MB_FC_WRITE_REG, MB_EX_SLAVE_FAILURE);
        return;
    }

    _reply = MB_REPLY_ECHO;
}

void Modbus::writeMultipleRegisters(uint8_t* frame,uint16_t startreg, uint16_t numoutputs, uint8_t bytecount) {
    //Check value
    if (numoutputs < 0x0001 || numoutputs > 0x007B || bytecount != 2 * numoutputs) {
        this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address (startreg...startreg + numregs)
    for (int k = 0; k < numoutputs; k++) {
        if (!this->searchRegister(HREG(startreg) + k)) {
            this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_ILLEGAL_ADDRESS);
            return;
        }
    }

    //Clean frame buffer
    free(_frame);
	_len = 5;
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_WRITE_REGS;
    _frame[1] = startreg >> 8;
    _frame[2] = startreg & 0x00FF;
    _frame[3] = numoutputs >> 8;
    _frame[4] = numoutputs & 0x00FF;

    uint16_t val;
    uint16_t i = 0;
	while(numoutputs--) {
        val = (word)frame[6+i*2] << 8 | (word)frame[7+i*2];
        this->Hreg(startreg + i, val);
        i++;
	}

    _reply = MB_REPLY_NORMAL;
}

#ifndef USE_HOLDING_REGISTERS_ONLY
void Modbus::readCoils(uint16_t startreg, uint16_t numregs) {
    //Check value (numregs)
    if (numregs < 0x0001 || numregs > 0x07D0) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address
    //Check only startreg. Is this correct?
    //When I check all registers in range I got errors in ScadaBR
    //I think that ScadaBR request more than one in the single request
    //when you have more then one datapoint configured from same type.
    if (!this->searchRegister(COIL(startreg))) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_ADDRESS);
        return;
    }

    //Clean frame buffer
    free(_frame);
	_len = 0;

    //Determine the message length = function type, byte count and
	//for each group of 8 registers the message length increases by 1
	_len = 2 + numregs/8;
	if (numregs%8) _len++; //Add 1 to the message length for the partial byte.

    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_READ_COILS;
    _frame[1] = _len - 2; //byte count (_len - function code and byte count)

    uint8_t bitn = 0;
    uint16_t totregs = numregs;
    uint16_t i;
	while (numregs--) {
        i = (totregs - numregs) / 8;
		if (this->Coil(startreg))
			bitSet(_frame[2+i], bitn);
		else
			bitClear(_frame[2+i], bitn);
		//increment the bit index
		bitn++;
		if (bitn == 8) bitn = 0;
		//increment the register
		startreg++;
	}

    _reply = MB_REPLY_NORMAL;
}

void Modbus::readInputStatus(uint16_t startreg, uint16_t numregs) {
    //Check value (numregs)
    if (numregs < 0x0001 || numregs > 0x07D0) {
        this->exceptionResponse(MB_FC_READ_INPUT_STAT, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address
    //*** See comments on readCoils method.
    if (!this->searchRegister(ISTS(startreg))) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_ADDRESS);
        return;
    }

    //Clean frame buffer
    free(_frame);
	_len = 0;

    //Determine the message length = function type, byte count and
	//for each group of 8 registers the message length increases by 1
	_len = 2 + numregs/8;
	if (numregs%8) _len++; //Add 1 to the message length for the partial byte.

    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_READ_INPUT_STAT, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_READ_INPUT_STAT;
    _frame[1] = _len - 2;

    uint8_t bitn = 0;
    uint16_t totregs = numregs;
    uint16_t i;
	while (numregs--) {
        i = (totregs - numregs) / 8;
		if (this->Ists(startreg))
			bitSet(_frame[2+i], bitn);
		else
			bitClear(_frame[2+i], bitn);
		//increment the bit index
		bitn++;
		if (bitn == 8) bitn = 0;
		//increment the register
		startreg++;
	}

    _reply = MB_REPLY_NORMAL;
}

void Modbus::readInputRegisters(uint16_t startreg, uint16_t numregs) {
    //Check value (numregs)
    if (numregs < 0x0001 || numregs > 0x007D) {
        this->exceptionResponse(MB_FC_READ_INPUT_REGS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address
    //*** See comments on readCoils method.
    if (!this->searchRegister(IREG(startreg))) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_ADDRESS);
        return;
    }

    //Clean frame buffer
    free(_frame);
	_len = 0;

	//calculate the query reply message length
	//for each register queried add 2 bytes
	_len = 2 + numregs * 2;

    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_READ_INPUT_REGS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_READ_INPUT_REGS;
    _frame[1] = _len - 2;

    uint16_t val;
    uint16_t i = 0;
	while(numregs--) {
        //retrieve the value from the register bank for the current register
        val = this->Ireg(startreg + i);
        //write the high byte of the register value
        _frame[2 + i * 2]  = val >> 8;
        //write the low byte of the register value
        _frame[3 + i * 2] = val & 0xFF;
        i++;
	}

    _reply = MB_REPLY_NORMAL;
}

void Modbus::writeSingleCoil(uint16_t reg, uint16_t status) {
    //Check value (status)
    if (status != 0xFF00 && status != 0x0000) {
        this->exceptionResponse(MB_FC_WRITE_COIL, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address and execute (reg exists?)
    if (!this->Coil(reg, (bool)status)) {
        this->exceptionResponse(MB_FC_WRITE_COIL, MB_EX_ILLEGAL_ADDRESS);
        return;
    }

    //Check for failure
    if (this->Coil(reg) != (bool)status) {
        this->exceptionResponse(MB_FC_WRITE_COIL, MB_EX_SLAVE_FAILURE);
        return;
    }

    _reply = MB_REPLY_ECHO;
}

void Modbus::writeMultipleCoils(uint8_t* frame,uint16_t startreg, uint16_t numoutputs, uint8_t bytecount) {
    //Check value
    uint16_t bytecount_calc = numoutputs / 8;
    if (numoutputs%8) bytecount_calc++;
    if (numoutputs < 0x0001 || numoutputs > 0x07B0 || bytecount != bytecount_calc) {
        this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address (startreg...startreg + numregs)
    for (int k = 0; k < numoutputs; k++) {
        if (!this->searchRegister(COIL(startreg) + k)) {
            this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_ADDRESS);
            return;
        }
    }

    //Clean frame buffer
    free(_frame);
	_len = 5;
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_WRITE_COILS;
    _frame[1] = startreg >> 8;
    _frame[2] = startreg & 0x00FF;
    _frame[3] = numoutputs >> 8;
    _frame[4] = numoutputs & 0x00FF;

    uint8_t bitn = 0;
    uint16_t totoutputs = numoutputs;
    uint16_t i;
	while (numoutputs--) {
        i = (totoutputs - numoutputs) / 8;
        this->Coil(startreg, bitRead(frame[6+i], bitn));
        //increment the bit index
        bitn++;
        if (bitn == 8) bitn = 0;
        //increment the register
        startreg++;
	}

    _reply = MB_REPLY_NORMAL;
}
#endif
bool Modbus::onGet(uint16_t address, cbModbus cb, uint16_t numregs) {
	TRegister* reg;
	bool atLeastOne = false;
	while (numregs > 0) {
		numregs--;
		reg = this->searchRegister(address + numregs);
		if (reg) {
			reg->get = cb;
			atLeastOne = true;
		}
	}
	return atLeastOne;
}
bool Modbus::onSet(uint16_t address, cbModbus cb, uint16_t numregs) {
	TRegister* reg;
	bool atLeastOne = false;
	while (numregs > 0) {
		numregs--;
		reg = this->searchRegister(address + numregs);
		if (reg) {
			reg->set = cb;
			atLeastOne = true;
		}
	}
	return atLeastOne;
}