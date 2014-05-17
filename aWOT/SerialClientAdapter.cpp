#include "SerialClientAdapter.h"
#include <Arduino.h>

SerialClientAdapter::SerialClientAdapter(HardwareSerial *serial) {
	this->serialPointer = serial;
}

int SerialClientAdapter::connect(IPAddress ip, uint16_t port) {
	return 1;
}
int SerialClientAdapter::connect(const char *host, uint16_t port) {
	return 1;
}

size_t SerialClientAdapter::write(uint8_t b) {
	return serial()->write(b);

}

size_t SerialClientAdapter::write(const char *str) {
	return serial()->write(str);
}

size_t SerialClientAdapter::write(const uint8_t *buf, size_t size) {
	return serial()->write(buf, size);
}

uint8_t SerialClientAdapter::connected() {
	long now = millis();
	while (now + 10 > millis()) {
		if (serial()->available()) {
			return serial()->available();
		}
	}
	return serial()->available();
}

int SerialClientAdapter::read() {
	return serial()->read();
}

void SerialClientAdapter::flush() {
	while (serial()->available()) {
		serial()->read();
	}
	return;
}

void SerialClientAdapter::stop() {
	while (serial()->available()) {
		serial()->read();
	}
	return;
}

int SerialClientAdapter::available() {
	long now = millis();
	while (now + 20 > millis()) {
		if (serial()->available()) {
			break;
		}
	}
	return serial()->available();
}

int SerialClientAdapter::print(const char* str) {
	return serial()->print(str);
}

int SerialClientAdapter::println(const char* str) {
	return serial()->println(str);
}
int SerialClientAdapter::println() {
	return serial()->println();
}

int SerialClientAdapter::print(uint16_t b) {
	return serial()->print(b);
}

int SerialClientAdapter::println(const int aHeaderValue) {
	return serial()->println(aHeaderValue);
}

int SerialClientAdapter::read(uint8_t *buf, size_t size) { //todo fix this
	if (serial()->available()) {
		long start = millis();
		int i = 0;
		while (start + 3 > millis() && i < 10) {
			if (serial()->available()) {
				start = millis();
				buf[i++] = serial()->read();
			}
		}
		return i;
	} else {
		return -1;
	}
}

int SerialClientAdapter::peek() {
	return serial()->peek();
}
