#include "EthernetClientAdapter.h"
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

EthernetClientAdapter::EthernetClientAdapter(EthernetClient client) {
	this->client = client;
}

size_t EthernetClientAdapter::write(uint8_t b) {
	return this->client.write(b);
}

size_t EthernetClientAdapter::write(const char *str) {
	return this->client.write(str);
}

size_t EthernetClientAdapter::write(const uint8_t *buf, size_t size) {
	return this->client.write(buf, size);
}

uint8_t EthernetClientAdapter::connected() {
	return this->client.connected();
}

int EthernetClientAdapter::read() {
	return this->client.read();
}

void EthernetClientAdapter::flush() {
	return this->client.flush();
}

void EthernetClientAdapter::stop() {
	return this->client.stop();
}

int EthernetClientAdapter::available() {
	return this->client.available();
}

int EthernetClientAdapter::print(const char* str) {
	return this->client.print(str);
}

int EthernetClientAdapter::println(const char* str) {
	return this->client.println(str);
}
int EthernetClientAdapter::println() {
	return this->client.println();
}

int EthernetClientAdapter::print(uint16_t b) {
	return this->client.print(b);
}

int EthernetClientAdapter::println(const int aHeaderValue) {
	return this->client.println(aHeaderValue);
}

int EthernetClientAdapter::read(uint8_t *buf, size_t size) {
	return this->client.read(buf, size);
}

int EthernetClientAdapter::peek() {
	return this->client.peek();
}

void EthernetClientAdapter::setClient(EthernetClient client) {
	this->client = client;
}
