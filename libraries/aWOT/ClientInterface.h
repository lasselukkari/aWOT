#ifndef CLIENTINTERFACE_H_
#define CLIENTINTERFACE_H_
#include "Arduino.h"
#include "IPAddress.h"

class ClientInterface {
public:
	virtual size_t write(uint8_t b);
	virtual size_t write(const char *str);
	virtual size_t write(const uint8_t *buf, size_t size);
	virtual uint8_t connected();
	virtual int read();
	virtual void flush();
	virtual void stop();
	virtual int available();
	virtual int print(const char* str);
	virtual int println(const char* str);
	virtual int println();
	virtual int print(uint16_t b);
	virtual int println(const int aHeaderValue);
	virtual int read(uint8_t *buf, size_t size);
	virtual int peek();
};

#endif /* CLIENTINTERFACE_H_ */
