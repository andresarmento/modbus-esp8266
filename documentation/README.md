# FAQ

---

## Where to get documentation for the library?

[API](API.md)
- [ModbusTCP](https://github.com/emelianov/modbus-esp8266/blob/master/examples/ESP-TCP)
- [ModbusRTU](https://github.com/emelianov/modbus-esp8266/blob/master/examples/RTU/#Modbus-RTU-Specific-API
- [Callbacks](https://github.com/emelianov/modbus-esp8266/blob/master/examples/Callback)
- [Modbus Security](https://github.com/emelianov/modbus-esp8266/blob/master/examples/TLS)

---

## Client work cycle diagram

![Client diagram](https://github.com/emelianov/modbus-esp8266/blob/master/resources/client.png)

---

## Server work cycle diagram 

![Server diagram](https://github.com/emelianov/modbus-esp8266/blob/master/resources/server.png)

---

## How to send `float` or `uint32_t` values?

---

## Value not read after `readCoil`/`readHreg`/etc

The library is designed to execute calls async way. That is `readHreg()` function just sends read request to Modbus server device and exits. Responce is processed (as suun as it's arrive) by `task()`. `task()` is also async and exits if data hasn't arrive yet.  

---

## When calling `readCoil`/`readHreg`/`writeHreg`/etc multiple times only first of them executed

---

## Transactional callback returns *0xE4* error

---

## If it's possible to create ModbusTCP to ModbusRTU pass through brodge?

Some ideas to implement full functional brodge may be taken from [this code](https://github.com/emelianov/modbus-esp8266/issues/101#issuecomment-755419095).
Very limited implementation is available in [examples](https://github.com/emelianov/modbus-esp8266/examples/bridge).

---

# Modbus Library for Arduino
### ModbusRTU, ModbusTCP and ModbusTCP Security

(c)2021 [Alexander Emelianov](mailto:a.m.emelianov@gmail.com)

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.