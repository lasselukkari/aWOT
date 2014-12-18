#ifndef ESP8266ClientAdapter_H_
#define ESP8266ClientAdapter_H_

#include <ClientInterface.h>
#include <Arduino.h>
#include <ESP8266.h>

class ESP8266ClientAdapter: public ClientInterface {
public:
	ESP8266ClientAdapter(ESP8266 *esp);
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

private:
	ESP8266 *esp;
};

#endif /* ESP8266ClientAdapter_H_ */
