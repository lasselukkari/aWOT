/*
 * CC3000ClientInterface.h
 *
 *  Created on: Sep 22, 2013
 *      Author: lasselukkari
 */

#ifndef CC3000CLIENTADAPTER_H_
#define CC3000CLIENTADAPTER_H_

#include <ClientInterface.h>
#include <CC3000/Adafruit_CC3000_Server.h>
#include <Arduino.h>

class CC3000ClientAdapter: public ClientInterface {
public:
	CC3000ClientAdapter(Adafruit_CC3000 * cc3000, Adafruit_CC3000_ClientRef client);
	int connect(IPAddress ip, uint16_t port);
	int connect(const char *host, uint16_t port);
	size_t write(uint8_t b);
	size_t write(const char *str);
	size_t write(const uint8_t *buf, size_t size);
	uint8_t connected();
	int read();
	void flush();
	void stop();
	int available();
	int print(const char* str);
	int println(const char* str);
	int println();
	int print(uint16_t b);
	int println(const int aHeaderValue);
	int read(uint8_t *buf, size_t size);
	int peek();
	void setClient(Adafruit_CC3000_ClientRef client);


private:
	Adafruit_CC3000 * cc3000;
	Adafruit_CC3000_ClientRef client;
	int peekBuffer;
};

#endif /* CC3000CLIENTADAPTER_H_ */

