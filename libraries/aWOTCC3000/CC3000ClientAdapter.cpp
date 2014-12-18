#include "CC3000/Adafruit_CC3000.h"
#include "CC3000ClientAdapter.h"
#include <Arduino.h>

CC3000ClientAdapter::CC3000ClientAdapter(Adafruit_CC3000 * cc3000,
		Adafruit_CC3000_ClientRef client) :
		client(client), cc3000(cc3000) {

}

size_t CC3000ClientAdapter::write(uint8_t b) {
	return this->client.write(b);
}

size_t CC3000ClientAdapter::write(const char *str) {

	return this->client.write(str, strlen(str));
}

size_t CC3000ClientAdapter::write(const uint8_t *buf, size_t size) {
	return this->client.write(buf, size);
}

uint8_t CC3000ClientAdapter::connected() {
	return this->client.connected();
}

int CC3000ClientAdapter::read() {
	if (peekBuffer) {
		int ret = (int) peekBuffer;
		peekBuffer = NULL;
		return ret;
	} else {
		return this->client.read();
	}

}

void CC3000ClientAdapter::flush() {
	while (this->client.available()) {
		this->client.read();
	}
	return;
}

void CC3000ClientAdapter::stop() {
	delay(10);
	this->client.close();
	return;
}

int CC3000ClientAdapter::available() {
	return this->client.available();
}

int CC3000ClientAdapter::print(const char* str) {
	return this->client.print(str);
}

int CC3000ClientAdapter::println(const char* str) {
	return this->client.println(str);
}
int CC3000ClientAdapter::println() {
	return this->client.println();
}

int CC3000ClientAdapter::print(uint16_t b) {
	return this->client.print(b);
}

int CC3000ClientAdapter::println(const int aHeaderValue) {
	return this->client.println(aHeaderValue);
}

int CC3000ClientAdapter::read(uint8_t *buf, size_t size) {

	if (peekBuffer) {
		buf[0] = (int) peekBuffer;
		buf++;
		peekBuffer = NULL;
		return this->client.read(buf, size - 1) + 1;
	} else {
		return this->client.read(buf, size);
	}
}

int CC3000ClientAdapter::peek() {
	if (peekBuffer) {
		return (int) peekBuffer;
	} else {
		this->peekBuffer = this->client.read();
		return this->peekBuffer;
	}

}

void CC3000ClientAdapter::setClient(Adafruit_CC3000_ClientRef client) {
	this->client = client;
}

