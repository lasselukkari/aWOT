
#ifndef SERIALSERVERADAPTER_H_
#define SERIALSERVERADAPTER_H_
#include <Arduino.h>
#include <ServerInterface.h>
#include <HardwareSerial.h>
#ifdef _SAM3XA_ // Arduino Due
#include <UARTClass.h>
#define SERIAL_CLASS UARTClass
//#elif __MK20DX128__ // Teensy 3.0
//#include <usb_serial.h>
//#define SERIAL_CLASS usb_serial_class
#else
#include <HardwareSerial.h>

#define SERIAL_CLASS HardwareSerial
#endif

#include <SerialClientAdapter.h>

class SerialServerAdapter: public ServerInterface {
public:
	SerialServerAdapter(SERIAL_CLASS *serial);
	ClientInterface *available();

private:
	SERIAL_CLASS *serialPointer;
	SerialClientAdapter * clientAdapter;
	virtual inline SERIAL_CLASS *serial() {
		return serialPointer;
	}
};

#endif /* SERIALSERVERADAPTER_H_ */
