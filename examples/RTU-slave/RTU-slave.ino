/*
  ModbusRTU ESP8266/ESP32
  Simple slave example
  
  (c)2019 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <ModbusRTU.h>

#define REGN 10
#define SLAVE_ID 1

ModbusRTU mb;

void setup() {
  mb.begin(&Serial, 9600, SERIAL_8N1);
  mb.slave(SLAVE_ID);
  mb.addHreg(REGN);
  mb.Hreg(REGN, 100);
}

void loop() {
  mb.task();
  yield();
}