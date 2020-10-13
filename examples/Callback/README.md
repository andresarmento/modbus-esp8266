# Modbus Library for Arduino
### ModbusRTU, ModbusTCP and ModbusTCP Security

# Callbacks

## [Register read/write callback](onSet/onSet.ino)

```c
bool onSetCoil(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onSetHreg(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onSetIsts(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onSetIreg(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
```

`address`   Address of register assign callback on
`cb`    Callback function
`numregs`   Count of sequental segisters assign this callback to

Assign callback function on register modify event. Multiple sequental registers can be affected by specifing `numregs` parameter.


```c
bool onGetCoil(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onGetHreg(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onGetIsts(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onGetIreg(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
```

`address`   Address of register assign callback on
`cb`    Callback function
`numregs`   Count of sequental segisters assign this callback to

Assign callback function on register query event. Multiple sequental registers can be affected by specifing `numregs` parameter.

```c
bool removeOnGetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnSetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnGetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnSetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnGetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnSetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnGetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnSetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
```

`address`   Address of register assign callback on
`cb`    Callback function or NULL to remove all the callbacks.
`numregs`   Count of sequental segisters remove this callback to.
Disconnect specific callback function or all callbacks of the type if cb=NULL.

## [Incoming request callback (applicable to server/slave)](Request/Request.ino)

```c
typedef Modbus::ResultCode (*cbRequest)(Modbus::FunctionCode fc, TAddress reg, uint16_t regCount);
bool onRequest(cbRequest cb = _onRequestDefault);
bool onRequestSuccess(cbRequest cb = _onRequestDefault);
```

Callback function receives Modbus function code, register type and offset (`TAddress` structure) and count of registers requested. The function should return `Modbus::EX_SUCCESS` to allow request processing or Modbus error code to block processing. This code will be returned to client/master.

## [Modbus TCP/TLS Incoming connection callback](onSet/onSet.ino)

```c
void onConnect(cbModbusConnect cb);
void onDisonnect(cbModbusConnect cb);
```

Modbus TCP Server Assign callback function on new incoming connection event.

```c
typedef bool (*cbModbusConnect)(IPAddress ip);
```

- `ip` Client's address of incomig connection source. `INADDR_NONE` for on disconnect callback.

## [Modbus TCP/TLS Transaction result](Transactional/Transactional.ino)


## Other related functions and defenitions