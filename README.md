# Modbus Master-Slave Library for ESP8266/ESP32

# v2.0.BETA

*Be careful using current version in production. Stable version from 'releases' page is recommended.*

This library allows your ESP8266/ESP32 to communicate via Modbus protocol. The Modbus is a master-slave protocol
used in industrial automation and can be used in other areas, such as home automation.

The Modbus generally uses serial RS-232 or RS-485 as physical layer (then called Modbus Serial) and TCP/IP via Ethernet or WiFi (Modbus IP).


In the current version the library allows the ESP8266/ESP32 operate async as a master and/or slave, supporting Modbus IP via wireless network. For more information about Modbus see:

http://pt.wikipedia.org/wiki/Modbus
http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
http://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf
http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf

## Features

* Supported platforms are
  * ESP8266
  * ESP32
* Operates as
  * slave
  * master
* Fully async operations. No loop locks.
* Supports Modbus IP (TCP)
* Reply exception messages for all supported functions
* Modbus functions supported:
  * 0x01 - Read Coils
  * 0x02 - Read Input Status (Read Discrete Inputs)
  * 0x03 - Read Holding Registers
  * 0x04 - Read Input Registers
  * 0x05 - Write Single Coil
  * 0x06 - Write Single Register
  * 0x0F - Write Multiple Coils
  * 0x10 - Write Multiple Registers
* Callbacks for
  * Master connect
  * Master/Slave disconnect
  * Read specific Register
  * Write specific Register
  * Slave transaction finish

## Notes:

1. When using Modbus IP the transport protocol is TCP (port 502).
2. The offsets for registers are 0-based. So be careful when setting your supervisory system or your testing software. For example, in [ScadaBR](http://www.scadabr.com.br) offsets are 0-based, then, a register configured as 100 in the library is set to 100 in ScadaBR. On the other hand, in the [CAS Modbus Scanner](http://www.chipkin.com/products/software/modbus-software/cas-modbus-scanner/) offsets are 1-based, so a register configured as 100 in library should be 101 in this software.
3. All type register's addresses are limited to 0..9999 for local and remote both.
4. For API specefication refer [API.md](https://github.com/emelianov/modbus-esp8266/blob/master/API.md)

## Last Changes

```diff
//Internal changes
+ Remove memory allocation checking for small blocks as anyway firmware will fail if so low memory available.
+ Change object's list implementation to *std::vector*
+ Modbus class refactoring
+ ModbusIP networking code refactoring and error reporting
+ Global registers storage to share between multiple Modbus* instances
//Public API changes
+ Modbus master implementation
+ Move enum constants. E.g. MB_FC_READ_COIL => Modbus::FC_READ_COIL
+ Back to marking private for onSet, onGet, addReg and Reg methods
+ Added callback-related eventSource method, onDisconnect and transaction result callbacks
// ToDo
- Extend register addressing to 1..65535
- ModbusSerial (over RS-485)
- Read/Write file records function
- Write mask register function
- Serial line-specific functions
```

## Contributions

https://github.com/emelianov/modbus-esp8266

a.m.emelianov@gmail.com

Original version:

http://github.com/andresarmento/modbus-esp8266

prof (at) andresarmento (dot) com

## License

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
