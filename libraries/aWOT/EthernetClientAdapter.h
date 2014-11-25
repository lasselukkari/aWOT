/*
 * EthernetClientInterface.h
 *
 *  Created on: Sep 22, 2013
 *      Author: lasselukkari
 */

#ifndef ETHERNETCLIENTADAPTER_H_
#define ETHERNETCLIENTADAPTER_H_

#include <ClientInterface.h>
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

class EthernetClientAdapter: public ClientInterface {
public:
	EthernetClientAdapter(EthernetClient client);
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
	void setClient(EthernetClient client);

private:
	EthernetClient client;
};

#endif /* ETHERNETCLIENTADAPTER_H_ */
