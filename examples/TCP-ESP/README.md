# Modbus Library for Arduino
### ModbusRTU, ModbusTCP and ModbusTCP Security

# ESP8266/ESP32 TCP Examples

## [Basic client](client.ino)

```c
void client();
bool connect(IPAddress ip, uint16_t port = MODBUSIP_PORT);
bool disconnect(IPAddress ip);
```

```c
uint16_t readCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readCoil(const char* host, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readCoil(String host, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeCoil(IPAddress ip, uint16_t offset, bool value, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readIsts(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeHreg(IPAddress ip, uint16_t offset, uint16_t value, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readIreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
```

- `ip` IP Address of Modbus server to get registers from
- `host` Hostname fo Modbus server to get registers from
- `offset` Address of first Modbus register to read/write

## [Client with blocking read operation](clientSync.ino)

```c
bool isTransaction(uint16_t id);
bool isConnected(IPAddress ip);
void dropTransactions();
```

## [Server]](server.ino)

```c
```