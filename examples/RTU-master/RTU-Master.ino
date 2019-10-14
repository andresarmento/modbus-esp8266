/*
  ModbusRTU ESP8266/ESP32
  Read multiple coils from slave device example
  
  (c)2019 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <ModbusRTU.h>
#ifdef ESP8266
 #include <SoftwareSerial.h>
 SoftwareSerial S(D1, D2, false, 256);
#endif

ModbusRTU mb;

bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  Serial.printf_P("Request result: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
  return true;
}

void setup() {
  Serial.begin(115200);
 #ifdef ESP8266
  S.begin(9600, SWSERIAL_8N1);
  mb.begin(&S);
 #else
  Serial1.begin(9600, SERIAL_8N1, 17, 18);
  mb.begin(&Serial1);
 #endif
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