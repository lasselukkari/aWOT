#include <Arduino.h>
#include "StreamServerAdapter.h"
#include "StreamClientAdapter.h"


StreamServerAdapter::StreamServerAdapter(Stream *stream) {
	this->streamPointer = stream;
	this->clientAdapter = new StreamClientAdapter(this->streamPointer);
}

ClientInterface *StreamServerAdapter::available() {
	if (stream()->available()) {
		delay(1);
		return this->clientAdapter;
	} else {
		return NULL;
	}

}
