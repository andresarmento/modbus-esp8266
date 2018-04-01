/*
    Modbus.h - Header for Modbus Base Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once

#include "Arduino.h"

#define MAX_REGS     32
#define MAX_FRAME   128
#define COIL_BASE     1
#define ISTS_BASE 10001
#define IREG_BASE 30001
#define HREG_BASE 40001
#define COIL(n) (n + COIL_BASE)
#define ISTS(n) (n + ISTS_BASE)
#define IREG(n) (n + IREG_BASE)
#define HREG(n) (n + HREG_BASE)
#define BIT_VAL(v) (v?0xFF00:0x0000)
#define BIT_BOOL(v) (v==0xFF00)
#define COIL_VAL(v) (v?0xFF00:0x0000)
#define COIL_BOOL(v) (v==0xFF00)
#define ISTS_VAL(v) (v?0xFF00:0x0000)
#define ISTS_BOOL(v) (v==0xFF00)

//Function Codes
enum modbusFunctionCode {
    MB_FC_READ_COILS       = 0x01, // Read Coils (Output) Status 0xxxx
    MB_FC_READ_INPUT_STAT  = 0x02, // Read Input Status (Discrete Inputs) 1xxxx
    MB_FC_READ_REGS        = 0x03, // Read Holding Registers 4xxxx
    MB_FC_READ_INPUT_REGS  = 0x04, // Read Input Registers 3xxxx
    MB_FC_WRITE_COIL       = 0x05, // Write Single Coil (Output) 0xxxx
    MB_FC_WRITE_REG        = 0x06, // Preset Single Register 4xxxx
    MB_FC_WRITE_COILS      = 0x0F, // Write Multiple Coils (Outputs) 0xxxx
    MB_FC_WRITE_REGS       = 0x10, // Write block of contiguous registers 4xxxx
    MB_FC_MASKWRITE_REG    = 0x16, // Not implemented
    MB_FC_READWRITE_REGS   = 0x17  // Not implemented
};

//Exception Codes
enum modbusResultCode {
    MB_EX_ILLEGAL_FUNCTION  = 0x01, // Function Code not Supported
    MB_EX_ILLEGAL_ADDRESS   = 0x02, // Output Address not exists
    MB_EX_ILLEGAL_VALUE     = 0x03, // Output Value not in Range
    MB_EX_SLAVE_FAILURE     = 0x04, // Slave Deive Fails to process request
    MB_EX_ACKNOWLEDGE       = 0x05, // Not used
    MB_EX_SLAVE_DEVICE_BUSY = 0x06  // Not used
};

//Reply Types
enum modbusReplyCode {
    MB_REPLY_OFF            = 0x01,
    MB_REPLY_ECHO           = 0x02,
    MB_REPLY_NORMAL         = 0x03,
    MB_REPLY_ERROR          = 0x04,
    MB_REPLY_UNEXPECTED     = 0x05
};

typedef struct TRegister;

// Callback function Type
typedef uint16_t (*cbModbus)(TRegister* reg, uint16_t val);

typedef struct TRegister {
    uint16_t address;
    uint16_t value;
    struct TRegister* next;
    cbModbus get;
    cbModbus set;
};

uint16_t cbDefault(TRegister* reg, uint16_t val);

class Modbus {
    private:
        TRegister* _regs_head = NULL;
        // All function assuming to process Modbus frame from Slave perspective
        // I.e. readRegisters fill frame with local regisers vales
        // writeRegisters sets local registers according to frame values
	    void readBits(uint16_t startreg, uint16_t numregs, modbusFunctionCode fn = MB_FC_READ_COILS);
	    void readWords(uint16_t startreg, uint16_t numregs, modbusFunctionCode fn = MB_FC_READ_REGS);
        void readRegisters(uint16_t startreg, uint16_t numregs) {
            readWords(HREG(startreg), numregs, MB_FC_READ_REGS);
        }
        void writeSingleRegister(uint16_t reg, uint16_t value, modbusFunctionCode fn = MB_FC_WRITE_REG);
        void writeMultipleRegisters(uint8_t* frame, uint16_t startreg, uint16_t numoutputs, uint8_t bytecount, modbusFunctionCode fn = MB_FC_WRITE_REGS);
        void exceptionResponse(modbusFunctionCode fn, modbusResultCode excode);
        void readCoils(uint16_t startreg, uint16_t numregs) {
            readBits(COIL(startreg), numregs, MB_FC_READ_COILS);
        }
        void readInputStatus(uint16_t startreg, uint16_t numregs) {
            readBits(ISTS(startreg), numregs, MB_FC_READ_INPUT_STAT);
        }
        void readInputRegisters(uint16_t startreg, uint16_t numregs) {
            readWords(IREG(startreg), numregs, MB_FC_READ_INPUT_REGS);
        }
        void writeSingleCoil(uint16_t reg, uint16_t status, modbusFunctionCode fn = MB_FC_WRITE_COIL);
        void writeMultipleCoils(uint8_t* frame, uint16_t startreg, uint16_t numoutputs, uint8_t bytecount, modbusFunctionCode fn = MB_FC_WRITE_COILS);

        TRegister* searchRegister(uint16_t addr);
        bool cbEnabled = true;
    
    protected:        
        uint8_t* _frame;
        uint8_t  _len;
        uint8_t  _reply;
        void receivePDU(uint8_t* frame);
        void responcePDU(uint8_t* frame);

    public:
        bool readSlave(uint16_t startreg, uint16_t numregs, modbusFunctionCode fn = MB_FC_READ_REGS);
        bool writeSlaveBits(uint16_t startreg, uint16_t numregs, modbusFunctionCode fn = MB_FC_WRITE_COILS);
        bool writeSlaveWords(uint16_t startreg, uint16_t numregs, modbusFunctionCode fn = MB_FC_WRITE_REGS);

        bool addReg(uint16_t address, uint16_t value = 0, uint16_t numregs = 1);
        bool Reg(uint16_t address, uint16_t value);
        uint16_t Reg(uint16_t address);

        bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1) {
            return addReg(HREG(offset), value, numregs);
        }
        bool Hreg(uint16_t offset, uint16_t value) {
            return Reg(HREG(offset), value);
        }
        uint16_t Hreg(uint16_t offset) {
            return Reg(HREG(offset));
        }
        bool addCoil(uint16_t offset, bool value = false, uint16_t numregs = 1) {
            return addReg(COIL(offset), COIL_VAL(value), numregs);
        }
        bool addIsts(uint16_t offset, bool value = false, uint16_t numregs = 1) {
            return addReg(ISTS(offset), ISTS_VAL(value), numregs);
        }
        bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1) {
            return addReg(IREG(offset), value, numregs);
        }
        bool Coil(uint16_t offset, bool value) {
            return Reg(COIL(offset), COIL_VAL(value));
        }
        bool Ists(uint16_t offset, bool value) {
            return Reg(ISTS(offset), ISTS_VAL(value));
        }
        bool Ireg(uint16_t offset, uint16_t value) {
            return Reg(IREG(offset), value);
        }
        bool Coil(uint16_t offset) {
            return COIL_BOOL(Reg(COIL(offset)));
        }
        bool Ists(uint16_t offset) {
            return ISTS_BOOL(Reg(ISTS(offset)));
        }
        uint16_t Ireg(uint16_t offset) {
            return Reg(IREG(offset));
        }
        
        void cbEnable(bool state = true);
        void cbDisable() {
            cbEnable(false);
        }
        bool onGet(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1);
        bool onSet(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1);

        bool onGetCoil(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onGet(COIL(offset), cb, numregs);
        }
        bool onSetCoil(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onSet(COIL(offset), cb, numregs);
        }
        bool onGetHreg(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onGet(HREG(offset), cb, numregs);
        }
        bool onSetHreg(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onSet(HREG(offset), cb, numregs);
        }
        bool onGetIsts(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onGet(ISTS(offset), cb, numregs);
        }
        bool onSetIsts(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onSet(ISTS(offset), cb, numregs);
        }
        bool onGetIreg(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onGet(IREG(offset), cb, numregs);
        }
        bool onSetIreg(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onSet(IREG(offset), cb, numregs);
        }
};