#include <ModbusRTU.h>
#include <StreamBuf.h>
#include "common.h"
#include "write.h"
#include "read.h"
#include "files.h"


uint8_t stage = 0;
uint16_t readHreg = 0;

#define SLAVE_ID 1
#define HREG_ID 10
#define HREG_VALUE 100

#define HREGS_ID 20
#define HREGS_COUNT 20

void setup() {
  Serial.begin(115200);
  Serial.println("ModbusRTU API test");
  delay(100);
  master.begin(&P1);
  master.master();
  slave.begin(&P2);
  slave.slave(SLAVE_ID);
  slave.addHreg(HREG_ID);

writeSingle(SLAVE_ID, HREG(HREG_ID), HREG_VALUE);
writeSingle(SLAVE_ID, COIL(HREG_ID), true);

writeMultiple(SLAVE_ID, HREG(HREG_ID), 10);
writeMultiple(SLAVE_ID, COIL(HREG_ID), 10);

readMultiple(SLAVE_ID, HREG(HREG_ID), 10);
readMultiple(SLAVE_ID, COIL(HREG_ID), 10);
readMultiple(SLAVE_ID, IREG(HREG_ID), 10);
readMultiple(SLAVE_ID, ISTS(HREG_ID), 10);

// Garbage read
  {
  bool Node_1_ackStatus = false;
  bool Node_2_ackStatus = false;
  slave.addIsts(100, true);
  slave.addIsts(101, true);
  Serial.print("Write garbage: ");
  if (!master.slave()) {
        master.readIsts(2, 100, &Node_1_ackStatus, 1, NULL);
        while (master.slave()) {
           master.task();
           slave.task();
           delay(1);
        }
        master.readIsts(SLAVE_ID, 100, &Node_1_ackStatus, 1, NULL);
        while (master.slave()) {
          master.task();
          slave.task();
          delay(1);
        }
        master.readIsts(SLAVE_ID, 101, &Node_2_ackStatus, 1, NULL);
        while (master.slave()) {
           master.task();
           while(P2.available())
              P2.write(P2.read());
           //slave.task();
           delay(1);
        }
        master.readIsts(SLAVE_ID, 101, &Node_2_ackStatus, 1, NULL);
        while (master.slave()) {
          master.task();
          slave.task();
          delay(1);
        }
  }
  if (Node_1_ackStatus && Node_2_ackStatus) {
    Serial.println(" PASSED");
  } else {
    Serial.println(" FAILED");
  }
  }
  {
    initFile();
    testFile();
  }
}
void loop() {
  yield();
}