# Modbus Library for ESP8266/ESP32

## API

### Add [multiple] regs

```c
bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1)
bool addCoil(uint16_t offset, bool value = false, uint16_t numregs = 1)
bool addIsts(uint16_t offset, bool value = false, uint16_t numregs = 1)
bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t nemregs = 1)
```

### Write regs

```c
bool Hreg(uint16_t offset, uint16_t value)
bool Coil(uint16_t offset, bool value)
bool Ists(uint16_t offset, bool value)
bool Ireg(uint16_t offset, uint16_t value)
```

### Read regs

```c
uint16_t Reg(uint16_t address)
uint16_t Hreg(uint16_t offset)
bool Coil(uint16_t offset)
bool Ists(uint16_t offset)
uint16_t Ireg(uint16_t offset)
```

### Callbacks

```c
void cbEnable(bool state = TRUE)
```

Callback generation control. Callback generation is enabled by default.

```c
void cbDisable()
```

Disable callback generation.

```c
void onConnect(cbModbusConnect cb)
```

*ModbusIP only.* Assign callback function on new incoming connection event.

```c
typedef bool (*cbModbusConnect)(IPAddress ip)
```

Connect event callback function definition. Client IP address is passed as argument.

```c
typedef uint16_t (*cbModbus)(TRegister* reg, uint16_t val)
```

Get/Set register callback function definition. Pointer to TRegister structure (see source for details) of the register and new value are passed as arguments.

```c
IPAddress eventSource()
```

*ModbusIP and ModbusMasterIP only.* Should be called from onGet/onSet callback function. Returns IP address of remote requesting operation or IPADDR_NONE for local.

```c
bool onSetCoil(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1)
bool onSetHreg(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1)
bool onSetIsts(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1)
bool onSetIreg(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1)
```

Assign callback function on register modify event. Multiple sequental registers can be affected by specifing `numregs` parameter. Call in `onSetCoil(regId)` form to disconnect callback.


```c
bool onGetCoil(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1)
bool onGetHreg(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1)
bool onGetIsts(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1)
bool onGetIreg(uint16_t address, cbModbus cb = cbDefault, uint16_t numregs = 1)
```

Assign callback function on register query event. Multiple sequental registers can be affected by specifing `numregs` parameter. Call in `onGet(regId)` form to disconnect callback.

### Macros

```c
#define COIL_VAL(v)
#define COIL_BOOL(v)
#define ISTS_VAL(v)
#define ISTS_BOOL(v)
```

### ModBus IP specific

```c
void begin()
void task()
```

### ModBus IP Master specific

```c
void pullCoils()
void pushIstss()
void pullIstss()
void pushHregs()
void pullHregs()
void pushIregs()
void pullIregs()
```

### Callback example

```c
ModbusIP mb;
bool coil = false; // Define external variable to get/set value
uint16_t cbCoilSet(TRegister* reg, uint16_t val) {	// 'reg' is pointer to reg to modify, 'val' is new register value
  Serial.print("Set query from ");
  Serial.println(mb.eventSource().toString());
  coil = COIL_BOOL(val);
  return val;	// Returns value to be saved to TRegister structure
}
uint16_t cbCoilGet(TRegister* reg, uint16_t val) {
  Serial.print("Get query from ");
  Serial.println(mb.eventSource().toString());
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
  mb.onSetCoil(COIL_NR, cbCoilSet); // Add callback on Coil COIL_NR value set
  mb.onGetCoil(COIL_NR, cbCoilGet); // Add callback on Coil COIL_NR value get
...
}
void loop() {
...
	mb.task();
...
}
```


## Contributions

https://github.com/emelianov/modbus-esp8266

a.m.emelianov@gmail.com

Original version:

http://github.com/andresarmento/modbus-esp8266

prof (at) andresarmento (dot) com

## License

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
