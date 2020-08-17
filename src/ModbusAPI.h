/*
    Modbus Library for Arduino
	Modbus public API implementation
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once
#include "Modbus.h"

template <typename TYPEID, class T>
class ModbusAPI : public T {
	public:
/*	// New API
	uint16_t write(TYPEID id, TAddress reg, uint16_t value, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t write(TYPEID id, TAddress reg, bool value, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t write(TYPEID id, TAddress reg, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t write(TYPEID id, TAddress reg, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t read(TYPEID id, TAddress reg, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t read(TYPEID id, TAddress reg, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);

	uint16_t push(TYPEID id, TAddress to, TAddress from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pull(TYPEID id, TAddress from, TAddress to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
*/
	// Legacy API
/*
	bool Hreg(uint16_t offset, uint16_t* value);
    bool Coil(uint16_t offset, bool* value);
    bool Ists(uint16_t offset, bool* value);
    bool Ireg(uint16_t offset, uint16_t* value);
*/
	bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1);
    bool addCoil(uint16_t offset, bool value = false, uint16_t numregs = 1);
    bool addIsts(uint16_t offset, bool value = false, uint16_t numregs = 1);
    bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1);

    bool Hreg(uint16_t offset, uint16_t value);
    bool Coil(uint16_t offset, bool value);
    bool Ists(uint16_t offset, bool value);
    bool Ireg(uint16_t offset, uint16_t value);

    bool Coil(uint16_t offset);
    bool Ists(uint16_t offset);
    uint16_t Ireg(uint16_t offset);
    uint16_t Hreg(uint16_t offset);

    bool removeCoil(uint16_t offset, uint16_t numregs = 1);
    bool removeIsts(uint16_t offset, uint16_t numregs = 1);
    bool removeIreg(uint16_t offset, uint16_t numregs = 1);
    uint16_t removeHreg(uint16_t offset, uint16_t numregs = 1);

    bool onGetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool onSetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool onGetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool onSetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool onGetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool onSetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool onGetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool onSetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);

    bool removeOnGetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool removeOnSetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool removeOnGetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool removeOnSetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool removeOnGetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool removeOnSetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool removeOnGetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
    bool removeOnSetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);

	uint16_t writeCoil(TYPEID id, uint16_t offset, bool value, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t writeCoil(TYPEID id, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readCoil(TYPEID id, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t writeHreg(TYPEID id, uint16_t offset, uint16_t value, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t writeHreg(TYPEID id, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readIsts(TYPEID id, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readHreg(TYPEID id, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readIreg(TYPEID id, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);

	uint16_t pushCoil(TYPEID id, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullCoil(TYPEID id, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullIsts(TYPEID id, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pushHreg(TYPEID id, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullHreg(TYPEID id, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullIreg(TYPEID id, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);

	uint16_t pullHregToIreg(TYPEID id, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullCoilToIsts(TYPEID id, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pushIstsToCoil(TYPEID id, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pushIregToHreg(TYPEID id, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
};

// FNAME	writeCoil, writeIsts, writeHreg, writeIreg
// REG		COIL, ISTS, HREG, IREG
// FUNC		Modbus function
// MAXNUM	Register count limit
// VALTYPE	bool, uint16_t
// VALUE	
#define IMPLEMENT_WRITEREG(FNAME, REG, FUNC, VALUE, VALTYPE) \
template <typename TYPEID, class T> \
uint16_t ModbusAPI<TYPEID, T>::FNAME(TYPEID ip, uint16_t offset, VALTYPE value, cbTransaction cb, uint8_t unit) { \
	this->readSlave(offset, VALUE(value), Modbus::FUNC); \
	return this->send(ip, REG(offset), cb, unit, nullptr, cb); \
}
IMPLEMENT_WRITEREG(writeCoil, COIL, FC_WRITE_COIL, COIL_VAL, bool)
IMPLEMENT_WRITEREG(writeHreg, HREG, FC_WRITE_REG, , uint16_t)

#define IMPLEMENT_WRITEREGS(FNAME, REG, FUNC, VALUE, MAXNUM, VALTYPE) \
template <typename TYPEID, class T> \
uint16_t ModbusAPI<TYPEID, T>::FNAME(TYPEID ip, uint16_t offset, VALTYPE* value, uint16_t numregs, cbTransaction cb, uint8_t unit) { \
	if (numregs < 0x0001 || numregs > MAXNUM) return false; \
	this->VALUE(REG(offset), offset, numregs, Modbus::FUNC, value); \
	return this->send(ip, REG(offset), cb, unit, nullptr, cb); \
}
IMPLEMENT_WRITEREGS(writeCoil, COIL, FC_WRITE_COILS, writeSlaveBits, 0x07D0, bool)
IMPLEMENT_WRITEREGS(writeHreg, HREG, FC_WRITE_REGS, writeSlaveWords, 0x007D, uint16_t)

#define IMPLEMENT_READREGS(FNAME, REG, FUNC, MAXNUM, VALTYPE) \
template <typename TYPEID, class T> \
uint16_t ModbusAPI<TYPEID, T>::FNAME(TYPEID ip, uint16_t offset, VALTYPE* value, uint16_t numregs, cbTransaction cb, uint8_t unit) { \
	if (numregs < 0x0001 || numregs > MAXNUM) return false; \
	this->readSlave(offset, numregs, Modbus::FUNC); \
	return this->send(ip, REG(offset), cb, unit, value); \
}
IMPLEMENT_READREGS(readCoil, COIL, FC_READ_COILS, 0x07D0, bool)
IMPLEMENT_READREGS(readHreg, HREG, FC_READ_REGS, 0x007D, uint16_t)
IMPLEMENT_READREGS(readIsts, ISTS, FC_READ_INPUT_STAT, 0x07D0, bool)
IMPLEMENT_READREGS(readIreg, IREG, FC_READ_INPUT_REGS, 0x007D, uint16_t)

#define IMPLEMENT_PULL(FNAME, REG, FUNC, MAXNUM) \
template <typename TYPEID, class T> \
uint16_t ModbusAPI<TYPEID, T>::FNAME(TYPEID ip, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb, uint8_t unit) { \
	if (numregs < 0x0001 || numregs > MAXNUM) return false; \
	this->addCoil(to, numregs); \
	this->readSlave(from, numregs, Modbus::FUNC); \
	return this->send(ip, REG(to), cb, unit); \
}
IMPLEMENT_PULL(pullCoil, COIL, FC_READ_COILS, 0x07D0)
IMPLEMENT_PULL(pullIsts, ISTS, FC_READ_INPUT_STAT, 0x07D0)
IMPLEMENT_PULL(pullHreg, HREG, FC_READ_REGS, 0x007D)
IMPLEMENT_PULL(pullIreg, IREG, FC_READ_INPUT_REGS, 0x007D)
IMPLEMENT_PULL(pullHregToIreg, IREG, FC_READ_REGS, 0x007D)
IMPLEMENT_PULL(pullCoilToIsts, ISTS, FC_READ_COILS, 0x07D0)

#define IMPLEMENT_PUSH(FNAME, REG, FUNC, MAXNUM) \
template <typename TYPEID, class T> \
uint16_t ModbusAPI<TYPEID, T>::FNAME(TYPEID ip, uint16_t to, uint16_t from, uint16_t numregs, cbTransaction cb, uint8_t unit) { \
	if (numregs < 0x0001 || numregs > MAXNUM) return false; \
	if (!this->searchRegister(REG(from))) return false; \
	this->writeSlaveWords(REG(from), to, numregs, Modbus::FUNC); \
	return this->send(ip, REG(from), cb, unit); \
}
IMPLEMENT_PUSH(pushCoil, COIL, FC_WRITE_COILS, 0x7D0)
IMPLEMENT_PUSH(pushHreg, HREG, FC_WRITE_REGS, 0x007D)
IMPLEMENT_PUSH(pushIregToHreg, IREG, FC_WRITE_REGS, 0x007D)
IMPLEMENT_PUSH(pushIstsToCoil, ISTS, FC_WRITE_COILS, 0x07D0)

template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::addHreg(uint16_t offset, uint16_t value, uint16_t numregs) {
    return this->addReg(HREG(offset), value, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::Hreg(uint16_t offset, uint16_t value) {
    return this->Reg(HREG(offset), value);
}
template <typename TYPEID, class T> \
uint16_t ModbusAPI<TYPEID, T>::Hreg(uint16_t offset) {
    return this->Reg(HREG(offset));
}
template <typename TYPEID, class T> \
uint16_t ModbusAPI<TYPEID, T>::removeHreg(uint16_t offset, uint16_t numregs) {
    return this->removeReg(HREG(offset), numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::addCoil(uint16_t offset, bool value, uint16_t numregs) {
    return this->addReg(COIL(offset), COIL_VAL(value), numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::addIsts(uint16_t offset, bool value, uint16_t numregs) {
    return this->addReg(ISTS(offset), ISTS_VAL(value), numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::addIreg(uint16_t offset, uint16_t value, uint16_t numregs) {
    return this->addReg(IREG(offset), value, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::Coil(uint16_t offset, bool value) {
    return this->Reg(COIL(offset), COIL_VAL(value));
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::Ists(uint16_t offset, bool value) {
    return this->Reg(ISTS(offset), ISTS_VAL(value));
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::Ireg(uint16_t offset, uint16_t value) {
    return this->Reg(IREG(offset), value);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::Coil(uint16_t offset) {
    return COIL_BOOL(this->Reg(COIL(offset)));
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::Ists(uint16_t offset) {
    return ISTS_BOOL(this->Reg(ISTS(offset)));
}
template <typename TYPEID, class T> \
uint16_t ModbusAPI<TYPEID, T>::Ireg(uint16_t offset) {
    return this->Reg(IREG(offset));
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::removeCoil(uint16_t offset, uint16_t numregs) {
    return this->removeReg(COIL(offset), numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::removeIsts(uint16_t offset, uint16_t numregs) {
    return this->removeReg(ISTS(offset), numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::removeIreg(uint16_t offset, uint16_t numregs) {
    return this->removeReg(IREG(offset), numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::onGetCoil(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->onGet(COIL(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::onSetCoil(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->onSet(COIL(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::onGetHreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->onGet(HREG(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::onSetHreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->onSet(HREG(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::onGetIsts(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->onGet(ISTS(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::onSetIsts(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->onSet(ISTS(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::onGetIreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->onGet(IREG(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::onSetIreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->onSet(IREG(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::removeOnGetCoil(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->removeOnGet(COIL(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::removeOnSetCoil(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->removeOnSet(COIL(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::removeOnGetHreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->removeOnGet(HREG(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::removeOnSetHreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->removeOnSet(HREG(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::removeOnGetIsts(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->removeOnGet(ISTS(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::removeOnSetIsts(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->removeOnSet(ISTS(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::removeOnGetIreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->removeOnGet(IREG(offset), cb, numregs);
}
template <typename TYPEID, class T> \
bool ModbusAPI<TYPEID, T>::removeOnSetIreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return this->removeOnSet(IREG(offset), cb, numregs);
}