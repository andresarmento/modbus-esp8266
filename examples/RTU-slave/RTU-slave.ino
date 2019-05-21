/*
  ModbusRTU ESP8266/ESP32
  Simple slave example
  
  (c)2019 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define REGN 10

SoftwareSerial S(D1, D2, false, 128);
ModbusRTU<SoftwareSerial> mb;

void setup() {
  Serial.begin(115200);
  mb.begin(&S, 9600);
  mb.slave(1);
  mb.addHreg(REGN);
  mb.Hreg(REGN, 100);
}

bool coils[20];

void loop() {
  mb.task();
  yield();
}