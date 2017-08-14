/*
    Modbus.h - Header for Modbus Base Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "Modbus.h"

uint16_t cbDefault(TRegister* reg, uint16_t val) {
	return val;
}

Modbus::Modbus() {
    _regs_head = 0;
    _regs_last = 0;
}

TRegister* Modbus::searchRegister(uint16_t address) {
    TRegister *reg = _regs_head;
    //if there is no register configured, bail
    if(reg == 0) return(0);
    //scan through the linked list until the end of the list or the register is found.
    //return the pointer.
    do {
        if (reg->address == address) return(reg);
        reg = reg->next;
	} while(reg);
	return(0);
}

void Modbus::addReg(uint16_t address, uint16_t value) {
    TRegister *newreg;

	newreg = (TRegister *) malloc(sizeof(TRegister));
	newreg->address = address;
	newreg->value		= value;
	newreg->get = cbDefault;
	newreg->set = cbDefault;
	newreg->next		= 0;

	if(_regs_head == 0) {
        _regs_head = newreg;
        _regs_last = _regs_head;
    } else {
        //Assign the last register's next pointer to newreg.
        _regs_last->next = newreg;
        //then make temp the last register in the list.
        _regs_last = newreg;
    }
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
        return(reg->get(reg, reg->value));
    else
        return(0);
}

void Modbus::addHreg(uint16_t offset, uint16_t value) {
    this->addReg(offset + HREG_BASE, value);
}

bool Modbus::Hreg(uint16_t offset, uint16_t value) {
    return Reg(offset + HREG_BASE, value);
}

uint16_t Modbus::Hreg(uint16_t offset) {
    return Reg(offset + HREG_BASE);
}

#ifndef USE_HOLDING_REGISTERS_ONLY
    void Modbus::addCoil(uint16_t offset, bool value) {
        this->addReg(offset + COIL_BASE, value?0xFF00:0x0000);
    }

    void Modbus::addIsts(uint16_t offset, bool value) {
        this->addReg(offset + ISTS_BASE, value?0xFF00:0x0000);
    }

    void Modbus::addIreg(uint16_t offset, uint16_t value) {
        this->addReg(offset + IREG_BASE, value);
    }

    bool Modbus::Coil(uint16_t offset, bool value) {
        return Reg(offset + COIL_BASE, value?0xFF00:0x0000);
    }

    bool Modbus::Ists(uint16_t offset, bool value) {
        return Reg(offset + ISTS_BASE, value?0xFF00:0x0000);
    }

    bool Modbus::Ireg(uint16_t offset, uint16_t value) {
        return Reg(offset + IREG_BASE, value);
    }

    bool Modbus::Coil(uint16_t offset) {
        if (Reg(offset + COIL_BASE) == 0xFF00) {
            return true;
        } else return false;
    }

    bool Modbus::Ists(uint16_t offset) {
        if (Reg(offset + ISTS_BASE) == 0xFF00) {
            return true;
        } else return false;
    }

    uint16_t Modbus::Ireg(uint16_t offset) {
        return Reg(offset + IREG_BASE);
    }
#endif


void Modbus::receivePDU(byte* frame) {
    byte fcode  = frame[0];
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

void Modbus::exceptionResponse(byte fcode, byte excode) {
    //Clean frame buffer
    free(_frame);
    _len = 2;
    _frame = (byte *) malloc(_len);
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
    if (!this->searchRegister(startreg + HREG_BASE)) {
        this->exceptionResponse(MB_FC_READ_REGS, MB_EX_ILLEGAL_ADDRESS);
        return;
    }


    //Clean frame buffer
    free(_frame);
	_len = 0;

	//calculate the query reply message length
	//for each register queried add 2 bytes
	_len = 2 + numregs * 2;

    _frame = (byte *) malloc(_len);
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

void Modbus::writeMultipleRegisters(byte* frame,uint16_t startreg, uint16_t numoutputs, byte bytecount) {
    //Check value
    if (numoutputs < 0x0001 || numoutputs > 0x007B || bytecount != 2 * numoutputs) {
        this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address (startreg...startreg + numregs)
    for (int k = 0; k < numoutputs; k++) {
        if (!this->searchRegister(startreg + HREG_BASE + k)) {
            this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_ILLEGAL_ADDRESS);
            return;
        }
    }

    //Clean frame buffer
    free(_frame);
	_len = 5;
    _frame = (byte *) malloc(_len);
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
    if (!this->searchRegister(startreg + 1)) {
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

    _frame = (byte *) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_READ_COILS;
    _frame[1] = _len - 2; //byte count (_len - function code and byte count)

    byte bitn = 0;
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
    if (!this->searchRegister(startreg + 10001)) {
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

    _frame = (byte *) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_READ_INPUT_STAT, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_READ_INPUT_STAT;
    _frame[1] = _len - 2;

    byte bitn = 0;
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
    if (!this->searchRegister(startreg + 30001)) {
        this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_ADDRESS);
        return;
    }

    //Clean frame buffer
    free(_frame);
	_len = 0;

	//calculate the query reply message length
	//for each register queried add 2 bytes
	_len = 2 + numregs * 2;

    _frame = (byte *) malloc(_len);
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

void Modbus::writeMultipleCoils(byte* frame,uint16_t startreg, uint16_t numoutputs, byte bytecount) {
    //Check value
    uint16_t bytecount_calc = numoutputs / 8;
    if (numoutputs%8) bytecount_calc++;
    if (numoutputs < 0x0001 || numoutputs > 0x07B0 || bytecount != bytecount_calc) {
        this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_VALUE);
        return;
    }

    //Check Address (startreg...startreg + numregs)
    for (int k = 0; k < numoutputs; k++) {
        if (!this->searchRegister(startreg + 1 + k)) {
            this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_ADDRESS);
            return;
        }
    }

    //Clean frame buffer
    free(_frame);
	_len = 5;
    _frame = (byte *) malloc(_len);
    if (!_frame) {
        this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_SLAVE_FAILURE);
        return;
    }

    _frame[0] = MB_FC_WRITE_COILS;
    _frame[1] = startreg >> 8;
    _frame[2] = startreg & 0x00FF;
    _frame[3] = numoutputs >> 8;
    _frame[4] = numoutputs & 0x00FF;

    byte bitn = 0;
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


