/*
  Modbus-Arduino Example - Hreg multiple Holding register debug code (Modbus IP ESP8266/ESP32)
  
  Original library
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino

  Current version
<<<<<<< HEAD
  (c)2018 Alexander Emelianov (a.m.emelianov@gmail.com)
=======
  (c)2017 Alexander Emelianov (a.m.emelianov@gmail.com)
>>>>>>> std-list
  https://github.com/emelianov/modbus-esp8266
*/

#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else	//ESP32
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

#define LEN 10

//ModbusIP object
ModbusIP mb;

// Callback function to read corresponding DI
uint16_t cbRead(TRegister* reg, uint16_t val) {
  Serial.print("Read. Reg RAW#: ");
  Serial.print(reg->address);
  Serial.print(" Old: ");
  Serial.print(reg->value);
  Serial.print(" New: ");
  Serial.println(val);
  return val;
}
// Callback function to write-protect DI
uint16_t cbWrite(TRegister* reg, uint16_t val) {
  Serial.print("Write. Reg RAW#: ");
  Serial.print(reg->address);
  Serial.print(" Old: ");
  Serial.print(reg->value);
  Serial.print(" New: ");
  Serial.println(val);
  return val;
}

// Callback function for client connect. Returns true to allow connection.
bool cbConn(IPAddress ip) {
  Serial.println(ip);
  return true;
}
 
void setup() {
<<<<<<< HEAD
 #ifdef ESP8266
  Serial.begin(74880);
 #else
  Serial.begin(115200);
 #endif
=======
  Serial.begin(74880);
>>>>>>> std-list
 
  WiFi.begin("ssid", "pass");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  mb.onConnect(cbConn);   // Add callback on connection event
  mb.begin();

<<<<<<< HEAD
  if (!mb.addHreg(0, 0xF0F0, LEN)) Serial.println("Error"); // Add Hregs
=======
  if (!mb.addHReg(0, 0xF0F0, LEN)) Serial.println("Error"); // Add Coils. The same as mb.addCoil(COIL_BASE, false, LEN)
>>>>>>> std-list
  mb.onGetHreg(0, cbRead, LEN); // Add callback on Coils value get
  mb.onSetHreg(0, cbWrite, LEN);
}

void loop() {
   //Call once inside loop() - all magic here
   mb.task();
<<<<<<< HEAD
   delay(100);
=======
   yield();
>>>>>>> std-list
}
