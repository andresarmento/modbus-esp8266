Modbus Library for ESP8266
==========================

This library allows your ESP8266 to communicate via Modbus protocol. The Modbus is a master-slave protocol
used in industrial automation and can be used in other areas, such as home automation.

The Modbus generally uses serial RS-232 or RS-485 as physical layer (then called Modbus Serial) and TCP/IP via Ethernet or WiFi (Modbus IP).

In the current version the library allows the ESP8266 operate as a slave, supporting Modbus IP via wireless network. For more information about Modbus see:

http://pt.wikipedia.org/wiki/Modbus http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
http://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf

Features
========

<ul>
<li>Operates as a slave</li>
<li>Supports Modbus IP (TCP, not keep-alive)</li>
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
</ul>

<b>Notes:</b>

1. When using Modbus IP the transport protocol is TCP (port 502) and the connection is terminated to each transmitted message, that is, is not a keep-alive type connection.

2. The offsets for registers are 0-based. So be careful when setting your supervisory system or your testing software. For example, in ScadaBR (http://www.scadabr.com.br)
offsets are 0-based, then, a register configured as 100 in the library is set to 100 in ScadaBR. On the other hand, in the CAS Modbus Scanner
(http://www.chipkin.com/products/software/modbus-software/cas-modbus-scanner/) offsets are 1-based, so a register configured as 100 in library should be 101 in this software.

3. Early in the library Modbus.h file there is an option to limit the operation
to the functions of Holding Registers, saving space in the program memory.
Just comment out the following line:

```
#define USE_HOLDING_REGISTERS_ONLY
```
Thus, only the following functions are supported:
<ul>
    <li>0x03 - Read Holding Registers</li>
    <li>0x06 - Write Single Register</li>
    <li>0x10 - Write Multiple Registers</li>
</ul>


How to
======

```
This README is under development, for now, see the examples of the library.
````

Contributions
=============
http://github.com/andresarmento/modbus-esp8266<br>
prof (at) andresarmento (dot) com

License
=======
The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.

