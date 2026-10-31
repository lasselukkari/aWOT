/*
  Arduino.h - Arduino compatibility layer for native Windows

  This file provides the necessary shims to use the ArduinoCore-API
  on Windows with MinGW. It includes the official Arduino headers
  and provides platform-specific implementations.
*/

#ifndef ARDUINO_H_
#define ARDUINO_H_

// ArduinoCore uses WString.h, so the SDK string.h resolves normally.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Platform-specific implementations (declared extern "C" to match Common.h)

// millis() - returns monotonic milliseconds since the first timing call
unsigned long millis(void);

// micros() - returns monotonic microseconds from the same origin as millis()
unsigned long micros(void);

void delay(unsigned long ms);

void yield(void);

char* itoa(int value, char* buffer, int base);
char* ltoa(long value, char* buffer, int base);
char* utoa(unsigned value, char* buffer, int base);
char* ultoa(unsigned long value, char* buffer, int base);

#ifdef __cplusplus
}
#endif

// dtostrf uses Arduino's signature; callers provide enough output storage.
#include "Client.h"
#include "Common.h"
#include "IPAddress.h"
#include "Print.h"
#include "Printable.h"
#include "Stream.h"
#include "WString.h"
#include "deprecated-avr-comp/avr/dtostrf.h"

// Bring Arduino classes into global namespace for aWOT compatibility
using arduino::Client;
using arduino::IPAddress;
using arduino::Print;
using arduino::Stream;

// Undefine system INADDR_NONE to avoid conflict with arduino::INADDR_NONE
#ifdef INADDR_NONE
#undef INADDR_NONE
#endif

class SerialClass : public Stream {
 public:
  void begin(unsigned long) {}
  void end() {}

  size_t write(uint8_t c) override;
  size_t write(const uint8_t* buffer, size_t size) override;
  void flush() override;
  int available() override { return 0; }
  int read() override { return -1; }
  int peek() override { return -1; }
};

extern SerialClass Serial;

#endif
