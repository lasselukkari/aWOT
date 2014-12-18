#include <Arduino.h>
#include "ESP8266ServerAdapter.h"
#include "ESP8266ClientAdapter.h"


ESP8266ServerAdapter::ESP8266ServerAdapter(ESP8266 *esp) {
	this->esp = esp;
	this->clientAdapter = new ESP8266ClientAdapter(this->esp);
}

ClientInterface *ESP8266ServerAdapter::available() {
	if (esp->available()) {
		return this->clientAdapter;
	} else {
		return NULL;
	}

}
