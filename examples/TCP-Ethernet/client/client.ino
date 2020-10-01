
/*
  ModbusTCP for W5x00 Ethernet library
  Basic Server code example

  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <SPI.h>
#include <Ethernet.h>
#include <ModbusEthernet.h>

// Enter a MAC address and IP address for your controller below.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177); // The IP address will be dependent on your local network:
ModbusEthernet mb;              // Declare ModbusTCP instance

void setup() {
  Serial.begin(115200);     // Open serial communications and wait for port to open
  while (!Serial) {}        // wait for serial port to connect. Needed for Leonardo only
  Ethernet.begin(mac, ip);  // start the Ethernet connection
  delay(1000);              // give the Ethernet shield a second to initialize
  mb.server();              // Act as Modbus TCP server
  mb.addReg(HREG(100));     // Add Holding register #100
}

void loop() {
  mb.task();                // Server Modbus TCP queries
  delay(50);
}