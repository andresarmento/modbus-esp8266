/*
    Modbus Library for Arduino
    ModbusTCP general implementation
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/

#pragma once
#include "Modbus.h"

// Callback function Type
typedef bool (*cbModbusConnect)(IPAddress ip);

typedef struct TTransaction {
	uint16_t	transactionId;
	uint32_t	timestamp;
	cbTransaction cb = nullptr;
	uint8_t*	_frame = nullptr;
	void*		data = nullptr;
	TAddress	startreg;
	Modbus::ResultCode forcedEvent = Modbus::EX_SUCCESS;	// EX_SUCCESS means no forced event here. Forced EX_SUCCESS is not possible.
	bool operator ==(const TTransaction &obj) const {
		    return transactionId == obj.transactionId;
	}
};

template <class SERVER, class CLIENT, int PORT>
class ModbusTCPTemplate : public Modbus {
	protected:
	typedef union MBAP_t {
		struct {
			uint16_t transactionId;
			uint16_t protocolId;
			uint16_t length;
			uint8_t	 unitId;
		};
		uint8_t  raw[7];
	};
	cbModbusConnect cbConnect = nullptr;
	cbModbusConnect cbDisconnect = nullptr;
	SERVER* tcpserver = nullptr;
	CLIENT* tcpclient[MODBUSIP_MAX_CLIENTS];
	std::vector<TTransaction> _trans;
	int16_t		transactionId = 0;  // Last started transaction. Increments on unsuccessful transaction start too.
	int8_t n = -1;
	bool autoConnectMode = false;
	uint16_t serverPort = 0;

	TTransaction* searchTransaction(uint16_t id);
	void cleanupConnections();	// Free clients if not connected
	void cleanupTransactions();	// Remove timedout transactions and forced event

	int8_t getFreeClient();    // Returns free slot position
	int8_t getSlave(IPAddress ip);
	int8_t getMaster(IPAddress ip);
	uint16_t send(IPAddress ip, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, void* data = nullptr, bool waitResponse = true);
	// Prepare and send ModbusIP frame. _frame buffer and _len should be filled with Modbus data
	// ip - slave ip address
	// startreg - first local register to save returned data to (miningless for write to slave operations)
	// cb - transaction callback function
	// unit - slave modbus unit id
	// data - if not null use buffer to save returned data instead of local registers
	public:
	ModbusTCPTemplate();
	~ModbusTCPTemplate();
	bool isTransaction(uint16_t id);
	bool isConnected(IPAddress ip);
	bool connect(IPAddress ip, uint16_t port = PORT);
	bool disconnect(IPAddress ip);
	// ModbusTCP
	void server(uint16_t port = PORT);
	// ModbusTCP depricated
	inline void slave(uint16_t port = PORT) { server(port); }	// Depricated
	inline void master() { client(); }	// Depricated
	inline void begin() { server(); }; 	// Depricated
	void client();
	void task();
	void onConnect(cbModbusConnect cb = nullptr);
	void onDisconnect(cbModbusConnect cb = nullptr);
	uint32_t eventSource() override;
	void autoConnect(bool enabled = true);
	void dropTransactions();
};

template <class SERVER, class CLIENT, int PORT>
ModbusTCPTemplate<SERVER, CLIENT, PORT>::ModbusTCPTemplate() {
	//_trans.reserve(MODBUSIP_MAX_TRANSACIONS);
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		tcpclient[i] = nullptr;
}

template <class SERVER, class CLIENT, int PORT>
void ModbusTCPTemplate<SERVER, CLIENT, PORT>::client() {

}

template <class SERVER, class CLIENT, int PORT>
void ModbusTCPTemplate<SERVER, CLIENT, PORT>::server(uint16_t port) {
	serverPort = port;
	tcpserver = new SERVER(serverPort);
	tcpserver->begin();
}

template <class SERVER, class CLIENT, int PORT>
bool ModbusTCPTemplate<SERVER, CLIENT, PORT>::connect(IPAddress ip, uint16_t port) {
	//cleanupConnections();
	if(getSlave(ip) != -1)
		return true;
	int8_t p = getFreeClient();
	if (p == -1)
		return false;
	tcpclient[p] = new CLIENT();
	return tcpclient[p]->connect(ip, port);
}

template <class SERVER, class CLIENT, int PORT>
uint32_t ModbusTCPTemplate<SERVER, CLIENT, PORT>::eventSource() {		// Returns IP of current processing client query
	if (n >= 0 && n < MODBUSIP_MAX_CLIENTS && tcpclient[n])
		return (uint32_t)tcpclient[n]->remoteIP();
	return (uint32_t)INADDR_NONE;
}

template <class SERVER, class CLIENT, int PORT>
TTransaction* ModbusTCPTemplate<SERVER, CLIENT, PORT>::searchTransaction(uint16_t id) {
	std::vector<TTransaction>::iterator it = std::find_if(_trans.begin(), _trans.end(), [id](TTransaction& trans){return trans.transactionId == id;});
   	if (it != _trans.end()) return &*it;
   	return nullptr;
}

template <class SERVER, class CLIENT, int PORT>
void ModbusTCPTemplate<SERVER, CLIENT, PORT>::task() {
	MBAP_t _MBAP;
	cleanupConnections();
	if (tcpserver) {
		while (tcpserver->hasClient()) {
			CLIENT* currentClient = new CLIENT(tcpserver->available());
			if (!currentClient || !currentClient->connected())
				continue;
			if (cbConnect == nullptr || cbConnect(currentClient->remoteIP())) {
				#ifdef MODBUSIP_UNIQUE_CLIENTS
				// Disconnect previous connection from same IP if present
				n = getMaster(currentClient->remoteIP());
				if (n != -1) {
					tcpclient[n]->flush();
					delete tcpclient[n];
					tcpclient[n] = nullptr;
				}
				#endif
				n = getFreeClient();
				if (n > -1) {
					tcpclient[n] = currentClient;
					continue; // while
				}
			}
			// Close connection if callback returns false or MODBUSIP_MAX_CLIENTS reached
			delete currentClient;
		}
	}
	for (n = 0; n < MODBUSIP_MAX_CLIENTS; n++) {
		if (!tcpclient[n]) continue;
		if (!tcpclient[n]->connected()) continue;
		uint32_t readStart = millis();
		while (millis() - readStart < MODBUSIP_MAX_READMS &&  tcpclient[n]->available() > sizeof(_MBAP)) {
			tcpclient[n]->readBytes(_MBAP.raw, sizeof(_MBAP.raw));	// Get MBAP
		
			if (__bswap_16(_MBAP.protocolId) != 0) {   // Check if MODBUSIP packet. __bswap is usless there.
				while (tcpclient[n]->available())	// Drop all incoming if wrong packet
					tcpclient[n]->read();
					continue;
			}
			_len = __bswap_16(_MBAP.length);
			_len--; // Do not count with last byte from MBAP
			if (_len > MODBUSIP_MAXFRAME) {	// Length is over MODBUSIP_MAXFRAME
				exceptionResponse((FunctionCode)tcpclient[n]->read(), EX_SLAVE_FAILURE);
				_len--;	// Subtract for read byte
				for (uint8_t i = 0; tcpclient[n]->available() && i < _len; i++)	// Drop rest of packet
					tcpclient[n]->read();
			} else {
				free(_frame);
				_frame = (uint8_t*) malloc(_len);
				if (!_frame) {
					exceptionResponse((FunctionCode)tcpclient[n]->read(), EX_SLAVE_FAILURE);
					for (uint8_t i = 0; tcpclient[n]->available() && i < _len; i++)	// Drop packet
						tcpclient[n]->read();
				} else {
					if (tcpclient[n]->readBytes(_frame, _len) < _len) {	// Try to read MODBUS frame
						exceptionResponse((FunctionCode)_frame[0], EX_ILLEGAL_VALUE);
						//while (tcpclient[n]->available())	// Drop all incoming (if any)
						//	tcpclient[n]->read();
					} else {
						if (tcpclient[n]->localPort() == serverPort) {
							// Process incoming frame as slave
							slavePDU(_frame);
						} else {
							// Process reply to master request
							_reply = EX_SUCCESS;
							TTransaction* trans = searchTransaction(__bswap_16(_MBAP.transactionId));
							if (trans) { // if valid transaction id
								if ((_frame[0] & 0x7F) == trans->_frame[0]) { // Check if function code the same as requested
									// Procass incoming frame as master
									masterPDU(_frame, trans->_frame, trans->startreg, trans->data);
								} else {
									_reply = EX_UNEXPECTED_RESPONSE;
								}
								if (trans->cb) {
									trans->cb((ResultCode)_reply, trans->transactionId, nullptr);
								}
								free(trans->_frame);
								//_trans.erase(std::remove(_trans.begin(), _trans.end(), *trans), _trans.end() );
								std::vector<TTransaction>::iterator it = std::find(_trans.begin(), _trans.end(), *trans);
								if (it != _trans.end())
									_trans.erase(it);
							}
						}
					}
				}
			}
			if (tcpclient[n]->localPort() != serverPort) _reply = REPLY_OFF;	// No replay if it was responce to master
			if (_reply != REPLY_OFF) {
				_MBAP.length = __bswap_16(_len+1);     // _len+1 for last byte from MBAP					
				size_t send_len = (uint16_t)_len + sizeof(_MBAP.raw);
				uint8_t sbuf[send_len];				
				memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
				memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
				tcpclient[n]->write(sbuf, send_len);
				tcpclient[n]->flush();
			}
			if (_frame) {
				free(_frame);
				_frame = nullptr;
			}
			_len = 0;
		}
	}
	n = -1;
	cleanupTransactions();
}

template <class SERVER, class CLIENT, int PORT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT, PORT>::send(IPAddress ip, TAddress startreg, cbTransaction cb, uint8_t unit, void* data, bool waitResponse) {
	MBAP_t _MBAP;
#ifdef MODBUSIP_MAX_TRANSACIONS
	if (_trans.size() >= MODBUSIP_MAX_TRANSACIONS) return false;
#endif
	int8_t p = getSlave(ip);
	if (p == -1 || !tcpclient[p]->connected())
		return autoConnectMode?connect(ip):false;
	transactionId++;
	if (!transactionId) transactionId = 1;
	_MBAP.transactionId	= __bswap_16(transactionId);
	_MBAP.protocolId	= __bswap_16(0);
	_MBAP.length		= __bswap_16(_len+1);     //_len+1 for last byte from MBAP
	_MBAP.unitId		= unit;
	size_t send_len = _len + sizeof(_MBAP.raw);
	uint8_t sbuf[send_len];
	memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
	memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
	if (tcpclient[p]->write(sbuf, send_len) != send_len)
		return false;
	tcpclient[p]->flush();
	if (waitResponse) {
		TTransaction tmp;
		tmp.transactionId = transactionId;
		tmp.timestamp = millis();
		tmp.cb = cb;
		tmp.data = data;	// BUG: Should data be saved? It may lead to memory leak or double free.
		tmp._frame = _frame;
		tmp.startreg = startreg;
		_trans.push_back(tmp);
		_frame = nullptr;
		_len = 0;
	}
	return transactionId;
}

template <class SERVER, class CLIENT, int PORT>
void ModbusTCPTemplate<SERVER, CLIENT, PORT>::onConnect(cbModbusConnect cb) {
	cbConnect = cb;
}

template <class SERVER, class CLIENT, int PORT>
void ModbusTCPTemplate<SERVER, CLIENT, PORT>::onDisconnect(cbModbusConnect cb) {
		cbDisconnect = cb;
}

template <class SERVER, class CLIENT, int PORT>
void ModbusTCPTemplate<SERVER, CLIENT, PORT>::cleanupConnections() {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
		if (tcpclient[i] && !tcpclient[i]->connected()) {
			//IPAddress ip = tcpclient[i]->remoteIP();
			//tcpclient[i]->stop();
			delete tcpclient[i];
			tcpclient[i] = nullptr;
			if (cbDisconnect && cbEnabled) 
				cbDisconnect(IPADDR_NONE);
		}
	}
}

template <class SERVER, class CLIENT, int PORT>
void ModbusTCPTemplate<SERVER, CLIENT, PORT>::cleanupTransactions() {
	for (auto it = _trans.begin(); it != _trans.end();) {
		if (millis() - it->timestamp > MODBUSIP_TIMEOUT || it->forcedEvent != Modbus::EX_SUCCESS) {
			Modbus::ResultCode res = (it->forcedEvent != Modbus::EX_SUCCESS)?it->forcedEvent:Modbus::EX_TIMEOUT;
			if (it->cb)
				it->cb(res, it->transactionId, nullptr);
			free(it->_frame);
			it = _trans.erase(it);
		} else
			it++;
	}
}

template <class SERVER, class CLIENT, int PORT>
int8_t ModbusTCPTemplate<SERVER, CLIENT, PORT>::getFreeClient() {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		if (!tcpclient[i])
			return i;
	return -1;
}

template <class SERVER, class CLIENT, int PORT>
int8_t ModbusTCPTemplate<SERVER, CLIENT, PORT>::getSlave(IPAddress ip) {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		if (tcpclient[i] && tcpclient[i]->connected() && tcpclient[i]->remoteIP() == ip && tcpclient[i]->localPort() != serverPort)
			return i;
	return -1;
}

template <class SERVER, class CLIENT, int PORT>
int8_t ModbusTCPTemplate<SERVER, CLIENT, PORT>::getMaster(IPAddress ip) {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		if (tcpclient[i] && tcpclient[i]->connected() && tcpclient[i]->remoteIP() == ip && tcpclient[i]->localPort() == serverPort)
			return i;
	return -1;
}

template <class SERVER, class CLIENT, int PORT>
bool ModbusTCPTemplate<SERVER, CLIENT, PORT>::isTransaction(uint16_t id) {
	return searchTransaction(id) != nullptr;
}

template <class SERVER, class CLIENT, int PORT>
bool ModbusTCPTemplate<SERVER, CLIENT, PORT>::isConnected(IPAddress ip) {
	int8_t p = getSlave(ip);
	return  p != -1 && tcpclient[p]->connected();
}

template <class SERVER, class CLIENT, int PORT>
void ModbusTCPTemplate<SERVER, CLIENT, PORT>::autoConnect(bool enabled) {
	autoConnectMode = enabled;
}

template <class SERVER, class CLIENT, int PORT>
bool ModbusTCPTemplate<SERVER, CLIENT, PORT>::disconnect(IPAddress ip) {
	int8_t p = getSlave(ip);
	if (p != -1) {
		delete tcpclient[p];
		tcpclient[p] = nullptr;
	}
	return true;
}

template <class SERVER, class CLIENT, int PORT>
void ModbusTCPTemplate<SERVER, CLIENT, PORT>::dropTransactions() {
	for (auto &t : _trans) t.forcedEvent = EX_CANCEL;
}

template <class SERVER, class CLIENT, int PORT>
ModbusTCPTemplate<SERVER, CLIENT, PORT>::~ModbusTCPTemplate() {
	free(_frame);
	dropTransactions();
	cleanupConnections();
	cleanupTransactions();
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
		delete tcpclient[i];
		tcpclient[i] = nullptr;
	}
}