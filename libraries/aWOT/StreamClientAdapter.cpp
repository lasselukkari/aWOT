#include "StreamClientAdapter.h"
#include <Arduino.h>

StreamClientAdapter::StreamClientAdapter(Stream *stream) {
	this->streamPointer = stream;
}

int StreamClientAdapter::connect(IPAddress ip, uint16_t port) {
	return 1;
}
int StreamClientAdapter::connect(const char *host, uint16_t port) {
	return 1;
}

size_t StreamClientAdapter::write(uint8_t b) {
	return stream()->write(b);

}

size_t StreamClientAdapter::write(const char *str) {
	return stream()->write(str);
}

size_t StreamClientAdapter::write(const uint8_t *buf, size_t size) {
	return stream()->write(buf, size);
}

uint8_t StreamClientAdapter::connected() {
	long now = millis();
	while (now + 10 > millis()) {
		if (stream()->available()) {
			return stream()->available();
		}
	}
	return stream()->available();
}

int StreamClientAdapter::read() {
	return stream()->read();
}

void StreamClientAdapter::flush() {
	while (stream()->available()) {
		stream()->read();
	}
	return;
}

void StreamClientAdapter::stop() {
	while (stream()->available()) {
		stream()->read();
	}
	return;
}

int StreamClientAdapter::available() {
	long now = millis();
	while (now + 20 > millis()) {
		if (stream()->available()) {
			break;
		}
	}
	return stream()->available();
}

int StreamClientAdapter::print(const char* str) {
	return stream()->print(str);
}

int StreamClientAdapter::println(const char* str) {
	return stream()->println(str);
}
int StreamClientAdapter::println() {
	return stream()->println();
}

int StreamClientAdapter::print(uint16_t b) {
	return stream()->print(b);
}

int StreamClientAdapter::println(const int aHeaderValue) {
	return stream()->println(aHeaderValue);
}

int StreamClientAdapter::read(uint8_t *buf, size_t size) { //todo fix this
	if (stream()->available()) {
		long start = millis();
		int i = 0;
		while (start + 3 > millis() && i < 10) {
			if (stream()->available()) {
				start = millis();
				buf[i++] = stream()->read();
			}
		}
		return i;
	} else {
		return -1;
	}
}

int StreamClientAdapter::peek() {
	return stream()->peek();
}
