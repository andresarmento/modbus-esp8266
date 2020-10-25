/*
  Modbus ESP8266/ESP32
  Simple ModbesRTU to ModbusIP bridge

  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <ModbusIP_ESP8266.h>
#include <ModbusRTU.h>

#define TO_REG 10
#define SLAVE_ID 1
#define PULL_ID 2
#define FROM_REG 20

ModbusRTU mb1;
ModbusIP mb2;

void setup() {
  Serial1.begin(9600, SERIAL_8N1);
  mb1.begin(&Serial1);
  mb1.master();
  mb2.server();
  mb2.addHreg(TO_REG);
}

void loop() {
  if(!mb1.slave())
    mb1.pullHreg(PULL_ID, FROM_REG, TO_REG);
  mb1.task();
  mb2.task();
  delay(50);
}