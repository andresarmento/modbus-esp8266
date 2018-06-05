/*
  Modbus-Arduino Example - Master (Modbus IP ESP8266)
  Control a Led on GPIO0 pin using Write Single Coil Modbus Function 

THIS IS NOT WORKING SAMPLE -- JUST DEVELOPNEMT VERSION

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
const int ledPin = 0; //GPIO0

//ModbusIP object
ModbusMasterIP mb;
ModbusIP slave;
uint16_t gc(TRegister* r, uint16_t v) {
  Serial.print("Set ger: ");
  Serial.println(v);
  return v;
}
void setup() {
  Serial.begin(74880);
 
  WiFi.begin("EW", "iMpress6264");
  
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
  mb.addCoil(LED_COIL);
  mb.onSetCoil(LED_COIL, gc);
  mb.connect(IPAddress(192, 168, 30, 116));
  mb.pullCoil(LED_COIL);

  slave.begin();
  slave.addCoil(LED_COIL);
  slave.onSetCoil(LED_COIL, gc);

}
 
void loop() {
   //Call once inside loop() - all magic here
  mb.get();
  slave.task();
   //Attach ledPin to LED_COIL register
  digitalWrite(ledPin, mb.Coil(LED_COIL));
  delay(100);
}
