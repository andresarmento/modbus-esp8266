/*
  ModbusRTU ESP8266/ESP32
  Read multiple coils from slave device example
  
  (c)2019 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <ModbusRTU.h>
#include <SoftwareSerial.h>

SoftwareSerial S(D1, D2, false, 128);
ModbusRTU mb;

bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  Serial.printf_P("Request result: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
  return true;
}

void setup() {
  Serial.begin(115200);
  mb.begin(&S, 9600);
  mb.master();
}

bool coils[20];

void loop() {
  if (!mb.slave()) {
    mb.readCoil(1, 1, coils, 20, cbWrite);
  }
  mb.task();
  yield();
}