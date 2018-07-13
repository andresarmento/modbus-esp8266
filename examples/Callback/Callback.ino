/*
  Modbus-Arduino Example - Test Led using callback (Modbus IP ESP8266/ESP32)
  Control a Led on D4 pin using Write Single Coil Modbus Function 
  Original library
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino

  Current version
  (c)2017 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else //ESP32
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

//Modbus Registers Offsets (0-65535)
const int LED_COIL = 100;
//Used Pins
#ifdef ESP8266
 const int ledPin = D4; // Builtin ESP8266 LED
#else
 const int ledPin = TX; // ESP32 TX LED
#endif
//ModbusIP object
ModbusIP mb;

// Callback function for write (set) Coil. Returns value to store.
uint16_t cbLed(TRegister* reg, uint16_t val) {
  //Attach ledPin to LED_COIL register
  digitalWrite(ledPin, COIL_BOOL(val));
  return val;
}

// Callback function for client connect. Returns true to allow connection.
bool cbConn(IPAddress ip) {
  Serial.println(ip);
  return true;
}
 
void setup() {
 #ifdef ESP8266
  Serial.begin(74880);
 #else
  Serial.begin(115200);
 #endif
  WiFi.begin("SID", "PASSWORD");
  
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

  pinMode(ledPin, OUTPUT);
  mb.addCoil(LED_COIL);       // Add Coil. The same as mb.addCoil(COIL_BASE, false, LEN)
  mb.onSetCoil(LED_COIL, cbLed); // Add callback on Coil LED_COIL value set
}

void loop() {
   //Call once inside loop() - all magic here
   mb.task();
   delay(100);
}
