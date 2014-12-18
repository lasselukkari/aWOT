#include "EthernetServerAdapter.h"
#include "EthernetClientAdapter.h"
#include <Arduino.h>
#include "EthernetServer.h"

EthernetServerAdapter::EthernetServerAdapter(EthernetServer * server) {
	this->server_obj = server;
	this->clientAdapter = new EthernetClientAdapter(NULL);
}

ClientInterface *EthernetServerAdapter::available() {

	EthernetClient client = server()->available();

	if (client.connected()) {
		this->clientAdapter->setClient(client);
		return this->clientAdapter;
	}

	else {
		return NULL;
	}
}
