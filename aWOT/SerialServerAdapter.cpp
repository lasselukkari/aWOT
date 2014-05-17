#include <Arduino.h>
#include "SerialServerAdapter.h"
#include "SerialClientAdapter.h"
#ifdef _SAM3XA_ // Arduino Due
#include <UARTClass.h>
#define SERIAL_CLASS UARTClass
//#elif __MK20DX128__ // Teensy 3.0,
//#include <usb_serial.h>
//#define SERIAL_CLASS usb_serial_class
#else
#include <HardwareSerial.h>
#define SERIAL_CLASS HardwareSerial
#endif

SerialServerAdapter::SerialServerAdapter(SERIAL_CLASS *serial) {
	this->serialPointer = static_cast<SERIAL_CLASS*>(serial);
	this->clientAdapter = new SerialClientAdapter(this->serialPointer);
}

ClientInterface *SerialServerAdapter::available() {
	if (serial()->available()) {
		delay(1);
		return this->clientAdapter;
	} else {
		return NULL;
	}

}
