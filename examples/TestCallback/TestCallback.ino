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
#else
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

//Modbus Registers Offsets (0-9999)
const int LED_COIL = 100;
//Used Pins
const int ledPin = D4; // Builtin ESP8266 LED

//ModbusIP object
ModbusIP mb;

// Callback function
uint16_t cbLed(TRegister* reg, uint16_t val) {
  //Attach ledPin to LED_COIL register
  digitalWrite(ledPin, (val == 0xFF00));
  return val;
}
 
void setup() {
  Serial.begin(74880);
 
  WiFi.begin("ssid", "pass");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mb.begin();

  pinMode(ledPin, OUTPUT);
  mb.addCoil(LED_COIL);     // Add Coil
  mb.onSet(COIL(LED_COIL), cbLed); // Add callback on Coil LED_COIL value set
}

void loop() {
   //Call once inside loop() - all magic here
   mb.task();
}
