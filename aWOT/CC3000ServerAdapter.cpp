#include "CC3000ServerAdapter.h"
#include "CC3000ClientAdapter.h"
#include <Arduino.h>

CC3000ServerAdapter::CC3000ServerAdapter(Adafruit_CC3000_Server * server) {
	this->server_obj = server;
	this->clientAdapter = new CC3000ClientAdapter(NULL, NULL);
}

ClientInterface *CC3000ServerAdapter::available() {

	Adafruit_CC3000_ClientRef client = server()->available();

	if (client.connected()) {

		this->clientAdapter->setClient(client);

		return this->clientAdapter;
	}

	else {
		return NULL;
	}
}
