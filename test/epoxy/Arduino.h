/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 *
 * Parts derived from the AVR Arduino SDK version 1.8.3
 * (https://github.com/arduino/ArduinoCore-avr).
 * Copyright (c) 2005-2013 Arduino Team
 *
 * Parts derived from the ESP8266 Arduino SDK version 2.7.4
 * (https://github.com/esp8266/Arduino).
 */

#ifndef EPOXY_DUINO_EPOXY_ARDUINO_H
#define EPOXY_DUINO_EPOXY_ARDUINO_H

// xx.yy.zz => xxyyzz (without leading 0)
#define EPOXY_DUINO_VERSION 10600
#define EPOXY_DUINO_VERSION_STRING "1.6.0"

#include <algorithm> // min(), max()
#include <cmath> // abs()
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "WString.h"
#include "WCharacter.h"
#include "Print.h"
#include "StdioSerial.h"
#if defined(EPOXY_CORE_ESP8266)
  #include "Esp.h"
#endif

// Used by digitalRead() and digitalWrite()
#define HIGH 0x1
#define LOW  0x0

// Used by pinMode() and other GPIO functions.
#if defined(EPOXY_CORE_AVR)
  #define INPUT 0x0
  #define OUTPUT 0x1
  #define INPUT_PULLUP 0x2
#endif
#if defined(EPOXY_CORE_ESP8266)
  #define INPUT             0x00
  #define INPUT_PULLUP      0x02
  #define INPUT_PULLDOWN_16 0x04 // PULLDOWN only possible for pin16
  #define OUTPUT            0x01
  #define OUTPUT_OPEN_DRAIN 0x03
  #define WAKEUP_PULLUP     0x05
  #define WAKEUP_PULLDOWN   0x07
  #define SPECIAL           0xF8 //uart0rx/tx uart1tx and hspi
  #define FUNCTION_0        0x08
  #define FUNCTION_1        0x18
  #define FUNCTION_2        0x28
  #define FUNCTION_3        0x38
  #define FUNCTION_4        0x48
#endif

// Various math constants.
#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define EULER 2.718281828459045235360287471352

#define SERIAL  0x0
#define DISPLAY 0x1

#define LSBFIRST 0
#define MSBFIRST 1

// Interrupt Modes
#if defined(EPOXY_CORE_AVR)
  #define CHANGE 1
  #define FALLING 2
  #define RISING 3
#endif
#if defined(EPOXY_CORE_ESP8266)
  #define RISING    0x01
  #define FALLING   0x02
  #define CHANGE    0x03
  #define ONLOW     0x04
  #define ONHIGH    0x05
  #define ONLOW_WE  0x0C
  #define ONHIGH_WE 0x0D
#endif

#define DEFAULT 1
#define EXTERNAL 0

// Timer stuff on ESP8266
#if defined(EPOXY_CORE_ESP8266)
  #define TIM_DIV1 	0 //80MHz (80 ticks/us - 104857.588 us max)
  #define TIM_DIV16	1 //5MHz (5 ticks/us - 1677721.4 us max)
  #define TIM_DIV265	3 //312.5Khz (1 tick = 3.2us - 26843542.4 us max)

  //timer int_types
  #define TIM_EDGE	0
  #define TIM_LEVEL	1

  //timer reload values
  #define TIM_SINGLE	0 //on interrupt routine you need to write a new value to start the timer again
  #define TIM_LOOP	1 //on interrupt the counter will start with the same value again

  #define timer1_read()           (T1V)
  #define timer1_enabled()        ((T1C & (1 << TCTE)) != 0)
  #define timer1_interrupted()    ((T1C & (1 << TCIS)) != 0)

  typedef void(*timercallback)(void);

  void timer1_isr_init(void);
  void timer1_enable(uint8_t divider, uint8_t int_type, uint8_t reload);
  void timer1_disable(void);
  void timer1_attachInterrupt(timercallback userFunc);
  void timer1_detachInterrupt(void);
  void timer1_write(uint32_t ticks); //maximum ticks 8388607

  // timer0 is a special CPU timer that has very high resolution but with
  // limited control.
  // it uses CCOUNT (ESP.GetCycleCount()) as the non-resetable timer counter
  // it does not support divide, type, or reload flags
  // it is auto-disabled when the compare value matches CCOUNT
  // it is auto-enabled when the compare value changes
  #define timer0_interrupted()    (ETS_INTR_PENDING() & (_BV(ETS_COMPARE0_INUM)))
  #define timer0_read() ((__extension__({uint32_t count;__asm__ __volatile__("esync; rsr %0,ccompare0":"=a" (count));count;})))
  #define timer0_write(count) __asm__ __volatile__("wsr %0,ccompare0; esync"::"a" (count) : "memory")

  void timer0_isr_init(void);
  void timer0_attachInterrupt(timercallback userFunc);
  void timer0_detachInterrupt(void);
#endif

#if defined(EPOXY_CORE_AVR)
  #define NOT_A_PIN 0
  #define NOT_A_PORT 0
#endif
#if defined(EPOXY_CORE_ESP8266)
  #define NOT_A_PIN -1
  #define NOT_A_PORT -1
#endif
#define NOT_AN_INTERRUPT -1
#define NOT_ON_TIMER 0

// Arduino defines min(), max(), abs(), and round() using c-preprocessor macros
// in the global namespace. But this breaks the EpoxyFS library, which depends
// on 3rd party Unix libraries, which in turn assume that these are functions
// that can be overloaded. For EpoxyDuino, instead of macros, let's use the
// versions defined by <algorithm> and <cmath> in the std:: namespace and lift
// them into the global namespace.
//
// #define min(a,b) ((a)<(b)?(a):(b))
// #define max(a,b) ((a)>(b)?(a):(b))
// #define abs(x) ((x)>0?(x):-(x))
// #define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
using std::min;
using std::max;
using std::abs;
using std::round;
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))

// Stub implementations
#define interrupts()
#define noInterrupts()

// Fake the CPU clock to 16MHz on Linxu or MacOS.
#define F_CPU 16000000
#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )

#define bit(b) (1UL << (b))
#if defined(EPOXY_CORE_ESP8266)
  #define _BV(b) (1UL << (b))
#endif

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

// bitToggle() is defined only on Arduino AVR as far as I can tell.
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

extern "C" {

typedef bool boolean;
typedef uint8_t byte;
typedef uint16_t word;

#if defined(EPOXY_CORE_ESP8266)
  void ets_intr_lock();
  void ets_intr_unlock();

  void preinit(void);
  void init(void);
  void initVariant(void);

  void optimistic_yield(uint32_t interval_us);
  void esp_yield(void);
#endif

void yield();

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void analogReference(uint8_t mode);
void analogWrite(uint8_t pin, int val);
#if defined(EPOXY_CORE_ESP8266)
  void analogWriteFreq(uint32_t freq);
  void analogWriteResolution(int res);
  void analogWriteRange(uint32_t range);
#endif

/**
 * Control the value that will be returned by `digitalRead(pin)` by setting it
 * to `val`, where `val` is either 0 or 1. This may be useful for testing
 * purposes. This works only if `pin < 32` because the underlying implementation
 * uses a `uint32_t` for storage. If the `pin` is greater than or equal to 32,
 * this function does nothing and `digitalRead(pin)` will return 0.
 *
 * This function is available only on EpoxyDuino. It is not a standard Arduino
 * function, so it is not available when compiling on actual hardware.
 */
void digitalReadValue(uint8_t pin, uint8_t val);

/**
 * Check the value that was set by `digitalWrite(pin, val)` by setting the interesting
 * pin argument, where the return value is either 0 or 1. This may be useful for testing
 * purposes. This works only if `pin < 32` because the underlying implementation
 * uses a `uint32_t` for storage. If the `pin` is greater than or equal to 32,
 * this function will return 0.
 *
 * This function is available only on EpoxyDuino. It is not a standard Arduino
 * function, so it is not available when compiling on actual hardware.
 */
uint8_t digitalWriteValue(uint8_t pin);

unsigned long millis();
unsigned long micros();
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);
unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode);
void detachInterrupt(uint8_t interruptNum);
#if defined(EPOXY_CORE_ESP8266)
  void attachInterruptArg(uint8_t pin, void (*)(void*), void* arg, int mode);
#endif

/** Provided in the client code's *.ino file. */
void setup();

/** Provided in the client code's *.ino file. */
void loop();

/** Calls epoxyduino_main() unless overriden by user */
int main(int argc, char** argv);

/** Copy of the argc parameter of main() as a global variable. */
extern int epoxy_argc;

/** Copy of the argv parameter of main() as a global variable. */
extern const char* const* epoxy_argv;

/**
 * Enable echoing of each character. By default echoing is turned off for
 * consistency with the behavior of the serial port of a real Arduino
 * microcontroller. However when running on a Unix machine inside a terminal
 * emulator, it is often more useful to enable echoing so that the user can see
 * the characters being typed. This function should be called in the setup(),
 * guarded by a `#if defined(EPOXY_DUINO)`.
 */
void enableTerminalEcho();

}

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0);
void noTone(uint8_t _pin);

// WMath prototypes
long random(long);
long random(long, long);
void randomSeed(unsigned long);
long map(long, long, long, long, long);
uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);
#if defined(EPOXY_CORE_ESP8266)
  long secureRandom(long);
  long secureRandom(long, long);
#endif

#if defined(EPOXY_CORE_ESP8266)
  inline void setTZ(const char* /*tz*/) {}

  inline void configTime(
      int /*timezone*/,
      int /*daylightOffset_sec*/,
      const char* /*server1*/,
      const char* /*server2*/ = nullptr,
      const char* /*server3*/ = nullptr) {}

  inline void configTime(
      const char* /*tz*/,
      const char* /*server1*/,
      const char* /*server2*/ = nullptr,
      const char* /*server3*/ = nullptr) {}

  // esp32 api compatibility
  inline void configTzTime(const char* tz, const char* server1,
      const char* server2 = nullptr, const char* server3 = nullptr)
  {
      configTime(tz, server1, server2, server3);
  }
#endif

#if defined(EPOXY_CORE_ESP8266)
  #include "pins_arduino_esp8266.h"
#endif
#if defined(EPOXY_CORE_AVR)
  #include "pins_arduino_avr.h"
#endif

#endif
