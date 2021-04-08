/*
  ModbusRTU ESP32
  Concurent thread example
  
  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <ModbusRTU.h>

#define REG1 1
#define REG2 2
#define SLAVE_ID 1

ModbusRTU mb;

xSemaphoreHandle xMutex;
Modbus::ResultCode err;

bool resCallback(Modbus::ResultCode event, uint16_t, void*) {
  err = event;
}

Modbus::ResultCode readSync(uint16_t Address, uint16_t start, uint16_t num, uint16_t* buf) {
  xSemaphoreTake(xMutex, portMAX_DELAY);
  if (mb.slave()){
    xSemaphoreGive(xMutex);
    return Modbus::EX_GENERAL_FAILURE;
  }
  Serial.printf("SlaveID: %d Hreg %d\n", Address, start);
  mb.readHreg(Address, start, buf, num, resCallback);
  while (mb.slave()) {
    vTaskDelay(1);
    mb.task();
  }
  Modbus::ResultCode res = err;
  xSemaphoreGive(xMutex);
  return res;
}

void loop1( void * pvParameters );
void loop2( void * pvParameters );

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, 19, 18);
  mb.begin(&Serial1, 17);
  mb.master();
  xMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(
                    loop1,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    10,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
 
 xTaskCreatePinnedToCore(
                    loop2,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */

}

uint16_t hregs1[10];
void loop1( void * pvParameters ){
  while(true) {
      delay(10);
      if (readSync(1, 1, 10, hregs1) != Modbus::EX_SUCCESS)
        Serial.print("Error 1");
  }
}

uint16_t hregs2[10];
void loop2( void * pvParameters ){
  while(true) {
      delay(100);
      if (readSync(1, 2, 10, hregs2) == Modbus::EX_SUCCESS)
        Serial.println("OK 2");
      else
        Serial.print("Error 2");
  }
}

void loop() {
  delay(100);
}