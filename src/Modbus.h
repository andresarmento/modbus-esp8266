/*
    Modbuc.h - Header for Modbus Base Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2018 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once

#include "Arduino.h"

//#define MB_GLOBAL_REGS
#define MB_MAX_REGS     32
#define MB_MAX_FRAME   128
#define MB_MAX_ADDRESS 9999
#define COIL_BASE     1
#define ISTS_BASE 10001
#define IREG_BASE 30001
#define HREG_BASE 40001
#define COIL(n) (n + COIL_BASE)
#define ISTS(n) (n + ISTS_BASE)
#define IREG(n) (n + IREG_BASE)
#define HREG(n) (n + HREG_BASE)
//#define COIL(n) {TAddress::COIL, n}
//#define ISTS(n) {TAddress::ISTS, n}
//#define IREG(n) {TAddress::IREG, n}
//#define HREG(n) {TAddress::HREG, n}
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

typedef uint16_t (*cbModbus)(TRegister* reg, uint16_t val); // Callback function Type
typedef struct TAddress {
    enum RegType {COIL, ISTS, IREG, HREG};
    RegType type;
    uint16_t address;
    bool operator==(const TAddress &obj) const { // TAddress == TAddress
	    return type == obj.type && address == obj.address;
	}
    TAddress& operator++() {     // ++TAddress
        address++;
        return *this;
    }
    TAddress  operator++(int) {  // TAddress++
        TAddress result(*this);
        address++;
        return result;
    }
    TAddress& operator+=(const int& inc) {  // TAddress += integer
        address += inc;
        return *this;
    }
    TAddress operator+(const int &inc) {    // TAddress + integer
        TAddress result(*this);
        result.address += inc;
        return result;
   }
};

typedef struct TRegister {
    //TAddress    address;
    uint16_t address;
    uint16_t value;
    cbModbus get;
    cbModbus set;
    bool operator ==(const TRegister &obj) const {
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
            FC_READ_FILE_REC    = 0x14, // Not implemented
            FC_WRITE_FILE_REC   = 0x15, // Not implemented
            FC_MASKWRITE_REG    = 0x16, // Not implemented
            FC_READWRITE_REGS   = 0x17  // Not implemented
        };
        //Exception Codes
        //Custom result codes used internally and for callbacks but never used for Modbus responce
        enum ResultCode {
            EX_SUCCESS              = 0x00, // Custom. No error
            EX_ILLEGAL_FUNCTION     = 0x01, // Function Code not Supported
            EX_ILLEGAL_ADDRESS      = 0x02, // Output Address not exists
            EX_ILLEGAL_VALUE        = 0x03, // Output Value not in Range
            EX_SLAVE_FAILURE        = 0x04, // Slave or Master Deice Fails to process request
            EX_ACKNOWLEDGE          = 0x05, // Not used
            EX_SLAVE_DEVICE_BUSY    = 0x06, // Not used
            EX_MEMORY_PARITY_ERROR  = 0x08, // Not used
            EX_PATH_UNAVAILABLE     = 0x0A, // Not used
            EX_DEVICE_FAILED_TO_RESPOND = 0x0B, // Not used
            EX_GENERAL_FAILURE      = 0xE1, // Custom. Unexpected master error
            EX_DATA_MISMACH         = 0xE2, // Custom. Inpud data size mismach
            EX_UNEXPECTED_RESPONSE  = 0xE3, // Custom. Returned result doesn't mach transaction
            EX_TIMEOUT              = 0xE4, // Custom. Operation not finished within reasonable time
            EX_CONNECTION_LOST      = 0xE5  // Custom. Connection with device lost
        };

        bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1) {
        #ifdef MB_MAX_ADDRESS
            if (offset > MB_MAX_ADDRESS) return false;
        #endif
            return addReg(HREG(offset), value, numregs);
        }
        bool Hreg(uint16_t offset, uint16_t value) {
            return Reg(HREG(offset), value);
        }
        uint16_t Hreg(uint16_t offset) {
            return Reg(HREG(offset));
        }
        uint16_t removeHreg(uint16_t offset) {
            return removeReg(HREG(offset));
        }
        bool addCoil(uint16_t offset, bool value = false, uint16_t numregs = 1) {
        #ifdef MB_MAX_ADDRESS
            if (offset > MB_MAX_ADDRESS) return false;
        #endif
            return addReg(COIL(offset), COIL_VAL(value), numregs);
        }
        bool addIsts(uint16_t offset, bool value = false, uint16_t numregs = 1) {
        #ifdef MB_MAX_ADDRESS
            if (offset > MB_MAX_ADDRESS) return false;
        #endif
            return addReg(ISTS(offset), ISTS_VAL(value), numregs);
        }
        bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1) {
        #ifdef MB_MAX_ADDRESS
            if (offset > MB_MAX_ADDRESS) return false;
        #endif
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
    private:
	    void readBits(uint16_t startreg, uint16_t numregs, FunctionCode fn);
	    void readWords(uint16_t startreg, uint16_t numregs, FunctionCode fn);
        
        void setMultipleBits(uint8_t* frame, uint16_t startreg, uint16_t numoutputs);
        void setMultipleWords(uint8_t* frame, uint16_t startreg, uint16_t numoutputs);
        
        void getMultipleBits(uint8_t* frame, uint16_t startreg, uint16_t numregs);
        void getMultipleWords(uint8_t* frame, uint16_t startreg, uint16_t numregs);

        TRegister* searchRegister(uint16_t addr);
    
    protected:
        //Reply Types
        enum ReplyCode {
            REPLY_OFF            = 0x01,
            REPLY_ECHO           = 0x02,
            REPLY_NORMAL         = 0x03,
            REPLY_ERROR          = 0x04,
            REPLY_UNEXPECTED     = 0x05
        };
    #ifndef MB_GLOBAL_REGS
        std::vector<TRegister> _regs;
    #endif
        uint8_t*  _frame = nullptr;
        uint16_t  _len = 0;
        uint8_t   _reply = 0;
        bool cbEnabled = true;
        void exceptionResponse(FunctionCode fn, ResultCode excode);
        void successResponce(uint16_t startreg, uint16_t numoutputs, FunctionCode fn);
        void slavePDU(uint8_t* frame);    //For Slave
        void masterPDU(uint8_t* frame, uint8_t* sourceFrame);   //For Master

        bool readSlave(uint16_t address, uint16_t numregs, FunctionCode fn);
        bool writeSlaveBits(uint16_t address, uint16_t startreg, uint16_t numregs, FunctionCode fn);
        bool writeSlaveWords(uint16_t address, uint16_t startreg, uint16_t numregs, FunctionCode fn);

        bool addReg(uint16_t address, uint16_t value = 0, uint16_t numregs = 1);
        bool Reg(uint16_t address, uint16_t value);
        uint16_t Reg(uint16_t address);
        bool removeReg(uint16_t address);   // Not implemented

        bool onGet(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1);
        bool onSet(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1);
};
