/*
    Modbus.h - Header for Modbus Base Library
    Copyright (C) 2014 André Sarmento Barbosa
                  2017 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "Arduino.h"

#ifndef MODBUS_H
#define MODBUS_H

#define MAX_REGS     32
#define MAX_FRAME   128
#define COIL_BASE     1
#define ISTS_BASE 10001
#define IREG_BASE 30001
#define HREG_BASE 40001
#define COIL(n) (n + COIL_BASE)
#define ISTS(n) (n + ISTS_BASE)
#define IREG(n) (n + IREG_BASE)
#define HERG(n) (n + HREG_BASE)
#define COIL_VAL(v) (v?0xFF00:0x0000)
#define COIL_BOOL(v) (v==0xFF00)

//#define USE_HOLDING_REGISTERS_ONLY

//Function Codes
enum {
    MB_FC_READ_COILS       = 0x01, // Read Coils (Output) Status 0xxxx
    MB_FC_READ_INPUT_STAT  = 0x02, // Read Input Status (Discrete Inputs) 1xxxx
    MB_FC_READ_REGS        = 0x03, // Read Holding Registers 4xxxx
    MB_FC_READ_INPUT_REGS  = 0x04, // Read Input Registers 3xxxx
    MB_FC_WRITE_COIL       = 0x05, // Write Single Coil (Output) 0xxxx
    MB_FC_WRITE_REG        = 0x06, // Preset Single Register 4xxxx
    MB_FC_WRITE_COILS      = 0x0F, // Write Multiple Coils (Outputs) 0xxxx
    MB_FC_WRITE_REGS       = 0x10, // Write block of contiguous registers 4xxxx
};

//Exception Codes
enum {
    MB_EX_ILLEGAL_FUNCTION = 0x01, // Function Code not Supported
    MB_EX_ILLEGAL_ADDRESS  = 0x02, // Output Address not exists
    MB_EX_ILLEGAL_VALUE    = 0x03, // Output Value not in Range
    MB_EX_SLAVE_FAILURE    = 0x04, // Slave Deive Fails to process request
};

//Reply Types
enum {
    MB_REPLY_OFF    = 0x01,
    MB_REPLY_ECHO   = 0x02,
    MB_REPLY_NORMAL = 0x03,
};

typedef struct TRegister;

// Callback function Type
typedef uint16_t (*cbModbus)(TRegister* reg, uint16_t val);

typedef struct TRegister {
    uint16_t address;
    uint16_t value;
    //uint16_t index;
    struct TRegister* next;
    cbModbus get;
    cbModbus set;
} TRegister;

uint16_t cbDefault(TRegister* reg, uint16_t val);

class Modbus {
    private:
        TRegister *_regs_head;
        TRegister *_regs_last;

        void readRegisters(uint16_t startreg, uint16_t numregs);
        void writeSingleRegister(uint16_t reg, uint16_t value);
        void writeMultipleRegisters(byte* frame,uint16_t startreg, uint16_t numoutputs, byte bytecount);
        void exceptionResponse(byte fcode, byte excode);
        #ifndef USE_HOLDING_REGISTERS_ONLY
            void readCoils(uint16_t startreg, uint16_t numregs);
            void readInputStatus(uint16_t startreg, uint16_t numregs);
            void readInputRegisters(uint16_t startreg, uint16_t numregs);
            void writeSingleCoil(uint16_t reg, uint16_t status);
            void writeMultipleCoils(byte* frame,uint16_t startreg, uint16_t numoutputs, byte bytecount);
        #endif

        TRegister* searchRegister(uint16_t addr);

        void addReg(uint16_t address, uint16_t value = 0);
        bool Reg(uint16_t address, uint16_t value);
        uint16_t Reg(uint16_t address);

    protected:
        uint8_t *_frame;
        uint8_t  _len;
        uint8_t  _reply;
        void receivePDU(byte* frame);

    public:
        Modbus();

        void addHreg(uint16_t offset, uint16_t value = 0);
        bool Hreg(uint16_t offset, uint16_t value);
        uint16_t Hreg(uint16_t offset);

        #ifndef USE_HOLDING_REGISTERS_ONLY
            void addCoil(uint16_t offset, bool value = false);
            void addIsts(uint16_t offset, bool value = false);
            void addIreg(uint16_t offset, uint16_t value = 0);

            bool Coil(uint16_t offset, bool value);
            bool Ists(uint16_t offset, bool value);
            bool Ireg(uint16_t offset, uint16_t value);

            bool Coil(uint16_t offset);
            bool Ists(uint16_t offset);
            uint16_t Ireg(uint16_t offset);
        #endif
        
        bool onGet(uint16_t address, cbModbus cb = cbDefault);
        bool onSet(uint16_t address, cbModbus cb = cbDefault);
};

#endif //MODBUS_H
