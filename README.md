# Modbus Library for ESP8266/ESP32

This library allows your ESP8266/ESP32 to communicate via Modbus protocol. The Modbus is a master-slave protocol
used in industrial automation and can be used in other areas, such as home automation.

The Modbus generally uses serial RS-232 or RS-485 as physical layer (then called Modbus Serial) and TCP/IP via Ethernet or WiFi (Modbus IP).

In the current version the library allows the ESP8266/ESP32 operate async as a master and/or slave, supporting Modbus IP via wireless network. For more information about Modbus see:

http://pt.wikipedia.org/wiki/Modbus http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
http://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf

## Features

<ul>
<li>Operates as a slave, master or both</li>
<li>Fully async operations. No loop locks.</li>
<li>Supports Modbus IP (TCP)</li>
<li>Reply exception messages for all supported functions</li>
<li>Modbus functions supported:</li>
<ul>
    <li>0x01 - Read Coils</li>
    <li>0x02 - Read Input Status (Read Discrete Inputs)</li>
    <li>0x03 - Read Holding Registers</li>
    <li>0x04 - Read Input Registers</li>
    <li>0x05 - Write Single Coil</li>
    <li>0x06 - Write Single Register</li>
    <li>0x0F - Write Multiple Coils</li>
    <li>0x10 - Write Multiple Registers</li>
</ul>
<li>Callbacks for</li>
<ul>
    <li> - Incoming IP connection</li>
    <li> - Read specific Register</li>
    <li> - Write specific Register</li>
</ul>
</ul>

<b>Notes:</b>

1. When using Modbus IP the transport protocol is TCP (port 502).

2. The offsets for registers are 0-based. So be careful when setting your supervisory system or your testing software. For example, in ScadaBR (http://www.scadabr.com.br)
offsets are 0-based, then, a register configured as 100 in the library is set to 100 in ScadaBR. On the other hand, in the CAS Modbus Scanner
(http://www.chipkin.com/products/software/modbus-software/cas-modbus-scanner/) offsets are 1-based, so a register configured as 100 in library should be 101 in this software.

## API

### Add [multiple] regs
```
bool addReg(uint16_t address, uint16_t value = 0, uint16_t numregs = 1)
bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1)
bool addCoil(uint16_t offset, bool value = false, uint16_t numregs = 1)
bool addIsts(uint16_t offset, bool value = false, uint16_t numregs = 1)
bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t nemregs = 1)
```
### Write regs
```
bool Reg(uint16_t address, uint16_t value)
bool Hreg(uint16_t offset, uint16_t value)
bool Coil(uint16_t offset, bool value)
bool Ists(uint16_t offset, bool value)
bool Ireg(uint16_t offset, uint16_t value)
```
### Read regs
```
uint16_t Reg(uint16_t address)
uint16_t Hreg(uint16_t offset)
bool Coil(uint16_t offset)
bool Ists(uint16_t offset)
uint16_t Ireg(uint16_t offset)
```
### Callbacks
```
bool onGet(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1)
bool onSet(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1)
void onConnect(cbModbusConnect cb)
typedef uint16_t (*cbModbus)(TRegister* reg, uint16_t val)
typedef bool (*cbModbusConnect)(IPAddress ip)
```
### Macros
```
#define COIL(n)
#define ISTS(n)
#define IREG(n)
#define HREG(n)
#define COIL_VAL(v)
#define COIL_BOOL(v)
#define ISTS_VAL(v)
#define ISTS_BOOL(v)
```
### ModBus IP specific
```
void begin()
void task()
```

### Callback example

```
bool coil = false; // Define external variable to get/set value
uint16_t cbCoilSet(TRegister* reg, uint16_t val) {	// 'reg' is pointer to reg to modify, 'val' is new register value
  coil = COIL_BOOL(val);
  return val;	// Returns value to be saved to TRegister structure
}
uint16_t cbCoilGet(TRegister* reg, uint16_t val) {
  return COIL_VAL(coil);	// Returns value to be returned to ModBus master as reply for current request
}
bool cbConn(IPAddress ip) {
	Serial.println(ip);
	return true;		// Return 'true' to allow connection or 'false' to drop connection
}
ModbusIP mb;	// ModbusIP object
void setup() {
...
  mb.onConnect(cbConn);   // Add callback on connection event
  mb.begin();
  mb.addCoil(COIL_NR);     // Add Coil
  mb.onSet(COIL(COIL_NR), cbCoilSet); // Add callback on Coil COIL_NR value set
  mb.onGet(COIL(COIL_NR), cbCoilGet); // Add callback on Coil COIL_NR value get
...
}
void loop() {
...
	mb.task();
...
}
```


## Contributions

https://github.com/emelianov/modbus-esp8266<br>
a.m.emelianov@gmail.com

Original version:<br>
http://github.com/andresarmento/modbus-esp8266<br>
prof (at) andresarmento (dot) com

## License

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.

