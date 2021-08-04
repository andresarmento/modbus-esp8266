/*
  Modbus Library for Arduino Example - Modbus RTU Firmware Update - ESP8266/ESP32
  Update target node to update.
  Receives firmware to upload and flashes it.
  
  (c)2021 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/
#include <ModbusRTU.h>
ModbusRTU rtu;
#if defined (ESP32)
 #include <Update.h>
 #define ESP32_SKETCH_SIZE 1310720
#endif
#if defined(ESP8266)
 #include <SoftwareSerial.h>
 // SoftwareSerial S(D1, D2, false, 256);

 // receivePin, transmitPin, inverse_logic, bufSize, isrBufSize
 // connect RX to D2 (GPIO4, Arduino pin 4), TX to D1 (GPIO5, Arduino pin 4)
 SoftwareSerial S(4, 5);
#endif

#define UPDATE_ENABLE 301
#define UPDATE_FILE 301
#define SLAVE_ID 1

uint16_t update_enable(TRegister* reg, uint16_t val) {
    uint32_t sketchSpace;
    if (rtu.Reg(reg->address)) {
        if (COIL_BOOL(val))
            return BIT_VAL(true);    
        Serial.print("O");
        if(Update.end(true)){ //true to set the size to the current progress
            Serial.println("Update Success. \nRebooting...");
        }
        return BIT_VAL(false);
    }
    #ifdef ESP8266
    sketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
   #else
    sketchSpace = ESP32_SKETCH_SIZE;
   #endif
    Serial.printf("Starting update. FW max sise: %ld\n", sketchSpace);
    if(!Update.begin(sketchSpace)) {
        Update.printError(Serial);
        return BIT_VAL(false);
    }
    return BIT_VAL(true);
}

// Expected arguments are:
// func = EX_WRITE_FILE_REC
// fileNum = UPDATE_FILE
// recNumber ignored
// recLength = data size (words)
// frame = data to write ptr 
Modbus::ResultCode handle_file(Modbus::FunctionCode func, uint16_t fileNum, uint16_t recNumber, uint16_t recLength, uint8_t* frame) {
    if (func != Modbus::FC_WRITE_FILE_REC)
        return Modbus::EX_ILLEGAL_FUNCTION;
    if (!rtu.Reg(COIL(UPDATE_ENABLE)))
        return Modbus::EX_ILLEGAL_VALUE;
    if (fileNum != UPDATE_FILE)
        return Modbus::EX_ILLEGAL_VALUE;
    Serial.print(".");
    if(Update.write(frame, recLength * 2) != recLength * 2){
      Update.printError(Serial);
    }
    return Modbus::EX_SUCCESS;
}

void setup() {
    Serial.begin(115200);
#if defined(ESP8266)
    S.begin(9600, SWSERIAL_8N1);
    rtu.begin(&S);
#else
    Serial1.begin(9600, SERIAL_8N1);
    rtu.begin(&Serial1);
 #endif
    rtu.server(SLAVE_ID);
    rtu.onFile(handle_file);
    rtu.addReg(COIL(UPDATE_ENABLE));
    rtu.onSet(COIL(UPDATE_ENABLE), update_enable);
}

void loop() {
    rtu.task();
    yield();
}