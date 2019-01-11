/*
  Modbus-Arduino Example - Master (Modbus IP ESP8266/ESP32)
  Read Holding Register from Slave device

  (c)2018 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

const int REG = 528;               // Modbus Hreg Offset (0-9999)
IPAddress remote(192, 168, 30, 13);  // Address of Modbus Slave device
const int LOOP_COUNT = 10;

ModbusIP mb;  //ModbusIP object

void setup() {
 #ifdef ESP8266
  Serial.begin(74880);
 #else
  Serial.begin(115200);
 #endif
 
  WiFi.begin("SSID", "PASSWORD");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mb.master();
}

uint16_t res = 0;
uint8_t show = LOOP_COUNT;

void loop() {
  if (mb.isConnected(remote)) {   // Check if connection to Modbus Slave is established
    mb.readHreg(remote, REG, &res);  // Initiate Read Coil from Modbus Slave
  } else {
    mb.connect(remote);           // Try to connect if no connection
  }
  mb.task();                      // Common local Modbus task
  delay(100);                     // Pulling interval
  if (show--) {                   // Display Slave register value one time per second (with default settings)
    Serial.println(res);
    show = LOOP_COUNT;
  }
}