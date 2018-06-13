/*
    Modbuc.h - Header for Modbus Base Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once

#include "Arduino.h"
#include <list>

#define MB_MAX_REGS     32
#define MB_MAX_FRAME   128
#define MB_FRAME_HEADER  6
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
#define IS_COIL(n) (n < ISTS_BASE)
#define IS_ISTS(n) (n >= ISTS_BASE && n < IREG_BASE)
#define IS_IREG(n) (n >= IREG_BASE && n < HREG_BASE)
#define IS_HREG(n) (n >= HREG_BASE)

typedef struct TRegister;

// Callback function Type
typedef uint16_t (*cbModbus)(TRegister* reg, uint16_t val);

typedef struct TRegister {
    uint16_t address;
    uint16_t value;
    cbModbus get;
    cbModbus set;
    bool operator <(const TRegister &obj) const
	    {
		    return address < obj.address;
	    }
    bool operator ==(const TRegister &obj) const
	    {
		    return address == obj.address;
	    }
};

uint16_t cbDefault(TRegister* reg, uint16_t val);

class Modbus {
    public:
        //Function Codes
        enum FunctionCode {
            FC_READ_COILS       = 0x01, // Read Coils (Output) Status
            FC_READ_INPUT_STAT  = 0x02, // Read Input Status (Discrete Inputs)
            FC_READ_REGS        = 0x03, // Read Holding Registers
            FC_READ_INPUT_REGS  = 0x04, // Read Input Registers
            FC_WRITE_COIL       = 0x05, // Write Single Coil (Output)
            FC_WRITE_REG        = 0x06, // Preset Single Register
            FC_WRITE_COILS      = 0x0F, // Write Multiple Coils (Outputs)
            FC_WRITE_REGS       = 0x10, // Write block of contiguous registers
            FC_MASKWRITE_REG    = 0x16, // Not implemented
            FC_READWRITE_REGS   = 0x17  // Not implemented
        };
        //Exception Codes
        enum ResultCode {
            EX_ILLEGAL_FUNCTION  = 0x01, // Function Code not Supported
            EX_ILLEGAL_ADDRESS   = 0x02, // Output Address not exists
            EX_ILLEGAL_VALUE     = 0x03, // Output Value not in Range
            EX_SLAVE_FAILURE     = 0x04, // Slave or Master Deice Fails to process request
            EX_ACKNOWLEDGE       = 0x05, // Not used
            EX_SLAVE_DEVICE_BUSY = 0x06  // Not used
        };

        inline bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1) {
            return addReg(HREG(offset), value, numregs);
        }
        inline bool Hreg(uint16_t offset, uint16_t value) {
            return Reg(HREG(offset), value);
        }
        inline uint16_t Hreg(uint16_t offset) {
            return Reg(HREG(offset));
        }
        inline bool addCoil(uint16_t offset, bool value = false, uint16_t numregs = 1) {
            return addReg(COIL(offset), COIL_VAL(value), numregs);
        }
        inline bool addIsts(uint16_t offset, bool value = false, uint16_t numregs = 1) {
            return addReg(ISTS(offset), ISTS_VAL(value), numregs);
        }
        inline bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1) {
            return addReg(IREG(offset), value, numregs);
        }
        inline bool Coil(uint16_t offset, bool value) {
            return Reg(COIL(offset), COIL_VAL(value));
        }
        inline bool Ists(uint16_t offset, bool value) {
            return Reg(ISTS(offset), ISTS_VAL(value));
        }
        inline bool Ireg(uint16_t offset, uint16_t value) {
            return Reg(IREG(offset), value);
        }
        inline bool Coil(uint16_t offset) {
            return COIL_BOOL(Reg(COIL(offset)));
        }
        inline bool Ists(uint16_t offset) {
            return ISTS_BOOL(Reg(ISTS(offset)));
        }
        inline uint16_t Ireg(uint16_t offset) {
            return Reg(IREG(offset));
        }
        
        void cbEnable(bool state = true);
        inline void cbDisable() {
            cbEnable(false);
        }
        
        inline bool onGetCoil(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onGet(COIL(offset), cb, numregs);
        }
        inline bool onSetCoil(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onSet(COIL(offset), cb, numregs);
        }
        inline bool onGetHreg(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onGet(HREG(offset), cb, numregs);
        }
        inline bool onSetHreg(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onSet(HREG(offset), cb, numregs);
        }
        inline bool onGetIsts(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onGet(ISTS(offset), cb, numregs);
        }
        inline bool onSetIsts(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onSet(ISTS(offset), cb, numregs);
        }
        inline bool onGetIreg(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onGet(IREG(offset), cb, numregs);
        }
        inline bool onSetIreg(uint16_t offset, cbModbus cb = cbDefault, uint16_t numregs = 1) {
            return onSet(IREG(offset), cb, numregs);
        }
    private:
        // All function assuming to process Modbus frame from Slave perspective
        // I.e. readRegisters fill frame with local regisers vales
        // writeRegisters sets local registers according to frame values
	    void readBits(uint16_t startreg, uint16_t numregs, FunctionCode fn);
	    void readWords(uint16_t startreg, uint16_t numregs, FunctionCode fn);
        void readRegisters(uint16_t startreg, uint16_t numregs) {
            readWords(HREG(startreg), numregs, FC_READ_REGS);
        }
        void writeSingleRegister(uint16_t reg, uint16_t value, FunctionCode fn = FC_WRITE_REG);
        void writeMultipleRegisters(uint8_t* frame, uint16_t startreg, uint16_t numoutputs, uint8_t bytecount, FunctionCode fn = FC_WRITE_REGS);
        inline void readCoils(uint16_t startreg, uint16_t numregs) {
            readBits(COIL(startreg), numregs, FC_READ_COILS);
        }
        inline void readInputStatus(uint16_t startreg, uint16_t numregs) {
            readBits(ISTS(startreg), numregs, FC_READ_INPUT_STAT);
        }
        inline void readInputRegisters(uint16_t startreg, uint16_t numregs) {
            readWords(IREG(startreg), numregs, FC_READ_INPUT_REGS);
        }
        void writeSingleCoil(uint16_t reg, uint16_t status, FunctionCode fn = FC_WRITE_COIL);
        void writeMultipleCoils(uint8_t* frame, uint16_t startreg, uint16_t numoutputs, uint8_t bytecount, FunctionCode fn = FC_WRITE_COILS);

        TRegister* searchRegister(uint16_t addr);

        bool cbEnabled = true;
    
    protected:
        //Reply Types
        enum ReplyCode {
            REPLY_OFF            = 0x01,
            REPLY_ECHO           = 0x02,
            REPLY_NORMAL         = 0x03,
            REPLY_ERROR          = 0x04,
            REPLY_UNEXPECTED     = 0x05
        };

        std::list<TRegister> _regs;
        //std::vector<TRegister> _regs;
        uint8_t* _frame;
        uint8_t  _len;
        uint8_t  _reply;
        void exceptionResponse(FunctionCode fn, ResultCode excode);
        void successResponce(uint16_t startreg, uint16_t numoutputs, FunctionCode fn);
        void receivePDU(uint8_t* frame);    //For Slave
        void responcePDU(uint8_t* frame);   //For Master

        bool readSlave(uint16_t startreg, uint16_t numregs, FunctionCode fn);
        bool writeSlaveBits(uint16_t startreg, uint16_t numregs, FunctionCode fn);
        bool writeSlaveWords(uint16_t startreg, uint16_t numregs, FunctionCode fn);

        bool addReg(uint16_t address, uint16_t value = 0, uint16_t numregs = 1);
        bool Reg(uint16_t address, uint16_t value);
        uint16_t Reg(uint16_t address);

        bool onGet(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1);
        bool onSet(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1);
};