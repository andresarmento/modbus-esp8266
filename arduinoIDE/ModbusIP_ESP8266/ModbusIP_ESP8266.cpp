/*
    ModbusIP_ESP8266.cpp - Source for Modbus IP ESP8266 Library
    Copyright (C) 2015 André Sarmento Barbosa
*/
#include "ModbusIP_ESP8266.h"

WiFiServer server(MODBUSIP_PORT);

ModbusIP::ModbusIP() {

}

void ModbusIP::config(const char* ssid, const char* password) {
	WiFi.begin(ssid, password);
	server.begin();
}

void ModbusIP::task() {
	WiFiClient client = server.available();

	int raw_len = 0;
	
    if (client) {
		if (client.connected()) {
		    for (int x = 0; x < 300; x++) { // Time to have data available
				if (client.available()) {
					while (client.available() > raw_len) {  //Computes data length
						raw_len = client.available();
						delay(1);
					}
					break;
				}
				delay(10);				
			}
		}
				
		if (raw_len > 7) {
			for (int i=0; i<7; i++)	_MBAP[i] = client.read(); //Get MBAP

			_len = _MBAP[4] << 8 | _MBAP[5];
			_len--; // Do not count with last byte from MBAP
			if (_MBAP[2] !=0 || _MBAP[3] !=0) return;   //Not a MODBUSIP packet
			if (_len > MODBUSIP_MAXFRAME) return;      //Length is over MODBUSIP_MAXFRAME
			_frame = (byte*) malloc(_len);
			
			raw_len = raw_len - 7;
			for (int i=0; i< raw_len; i++)	_frame[i] = client.read(); //Get Modbus PDU
			
			this->receivePDU(_frame);
			client.flush();

			if (_reply != MB_REPLY_OFF) {
			    //MBAP
				_MBAP[4] = (_len+1) >> 8;     //_len+1 for last byte from MBAP
				_MBAP[5] = (_len+1) & 0x00FF;
				
				size_t send_len = (unsigned int)_len + 7;
				uint8_t sbuf[send_len];
				
				for (int i=0; i<7; i++)	    sbuf[i] = _MBAP[i];
				for (int i=0; i<_len; i++)	sbuf[i+7] = _frame[i];

				client.write(sbuf, send_len);
			}

			client.stop();
			free(_frame);
			_len = 0;
		}
	}
}
	/*
    uint8_t buffer[128] = {0};
    uint8_t mux_id;
    uint32_t len = _wifi->recv(&mux_id, buffer, sizeof(buffer), 100);

    if (len > 0) {
        int i = 0;
        while (i < 7) {
            _MBAP[i] = buffer[i];
             i++;
        }

        _len = _MBAP[4] << 8 | _MBAP[5];
        _len--; // Do not count with last byte from MBAP
        if (_MBAP[2] !=0 || _MBAP[3] !=0) return;   //Not a MODBUSIP packet
        if (_len > MODBUSIP_MAXFRAME) return;      //Length is over MODBUSIP_MAXFRAME

        _frame = (byte*) malloc(_len);
        i = 0;
        while (i < _len){
            _frame[i] = buffer[7+i];  //Forget MBAP and take just modbus pdu
            i++;
        }

        this->receivePDU(_frame);

        if (_reply != MB_REPLY_OFF) {
            //MBAP
            _MBAP[4] = _len >> 8;
            _MBAP[5] = _len | 0x00FF;
            buffer[4] = _MBAP[4];
            buffer[5] = _MBAP[5];

            i = 0;
            while (i < _len){
                buffer[i+7] = _frame[i];
                i++;
            }
            _wifi->send(mux_id, buffer, _len + 7);
            _wifi->releaseTCP(mux_id);
        }

        free(_frame);
        _len = 0;
    }

}
*/
