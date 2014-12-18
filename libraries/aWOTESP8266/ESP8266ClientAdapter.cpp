#include "ESP8266ClientAdapter.h"
#include <Arduino.h>

ESP8266ClientAdapter::ESP8266ClientAdapter(ESP8266 *esp) {
	this->esp = esp;
}

size_t ESP8266ClientAdapter::write(uint8_t b) {
	return this->esp->write(b);

}

size_t ESP8266ClientAdapter::write(const char *str) {
	return this->esp->write(str);
}

size_t ESP8266ClientAdapter::write(const uint8_t *buf, size_t size) {
	return this->esp->write(buf, size);
}

uint8_t ESP8266ClientAdapter::connected() {
	return this->esp->connected();
}

int ESP8266ClientAdapter::read() {
	return this->esp->read();
}

void ESP8266ClientAdapter::flush() {
	
		this->esp->flush();
	
	return;
}

void ESP8266ClientAdapter::stop() {

		this->esp->stop();
	
	return;
}

int ESP8266ClientAdapter::available() {
	long now = millis();
	while (now + 20 > millis()) {
		if (this->esp->available()) {
			break;
		}
	}
	return this->esp->available();
}

int ESP8266ClientAdapter::print(const char* str) {
	return this->esp->print(str);
}

int ESP8266ClientAdapter::println(const char* str) {
	return this->esp->println(str);
}
int ESP8266ClientAdapter::println() {
	return this->esp->println();
}

int ESP8266ClientAdapter::print(uint16_t b) {
	return this->esp->print(b);
}

int ESP8266ClientAdapter::println(const int aHeaderValue) {
	return this->esp->println(aHeaderValue);
}

int ESP8266ClientAdapter::read(uint8_t *buf, size_t size) { //todo fix this
	if (this->esp->available()) {
		long start = millis();
		int i = 0;
		while (start + 3 > millis() && i < 10) {
			if (this->esp->available()) {
				start = millis();
				buf[i++] = this->esp->read();
			}
		}
		return i;
	} else {
		return -1;
	}
}

int ESP8266ClientAdapter::peek() {
	return this->esp->peek();
}
