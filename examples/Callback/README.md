# Modbus Library for Arduino
### ModbusRTU, ModbusTCP and ModbusTCP Security

# Callbacks

There are three types of calbacks are available

## Register read/write callback

## Incoming request callback (applicable to server/slave)

```c
typedef Modbus::ResultCode (*cbRequest)(Modbus::FunctionCode fc, TAddress reg, uint16_t regCount);
bool onRequest(cbRequest cb = _onRequestDefault);
bool onRequestSuccess(cbRequest cb = _onRequestDefault);
```

Callback function receives Modbus function code, register type and offset (TAddress structure) and count of registers requested. The function should return Modbus::EX_SUCCESS to allow request processing or Modbus error code to block processing. This code will be returned to client/master.

[Example](Request/Request.ino)

## Incoming connection callback (ModbusTCP/TLS only)