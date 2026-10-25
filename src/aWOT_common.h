#ifndef AWOT_COMMON_H_
#define AWOT_COMMON_H_

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#include "Client.h"

#if defined(AWOT_NO_STD_FUNCTION_MIDDLEWARE) && defined(STD_FUNCTION_MIDDLEWARE)
#error "Conflicting aWOT middleware configuration"
#endif
#if !defined(AWOT_NO_STD_FUNCTION_MIDDLEWARE)
#ifndef STD_FUNCTION_MIDDLEWARE
#if defined(ESP8266) || defined(ESP32)
#define STD_FUNCTION_MIDDLEWARE
#elif defined(__has_include)
#if __has_include(<functional>)
#define STD_FUNCTION_MIDDLEWARE
#endif
#endif
#endif
#ifdef STD_FUNCTION_MIDDLEWARE
#include <functional>
#endif
#endif

#define CRLF "\r\n"

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__) || \
    defined(__AVR_ATmega16U4__) || defined(_AVR_ATmega328__)
#define LOW_MEMORY_MCU
#endif

#ifndef SERVER_URL_BUFFER_SIZE
#if defined(LOW_MEMORY_MCU)
#define SERVER_URL_BUFFER_SIZE 64
#else
#define SERVER_URL_BUFFER_SIZE 256
#endif
#endif

#ifndef SERVER_PUSHBACK_BUFFER_SIZE
#if defined(LOW_MEMORY_MCU)
#define SERVER_PUSHBACK_BUFFER_SIZE 32
#else
#define SERVER_PUSHBACK_BUFFER_SIZE 128
#endif
#endif

#if SERVER_PUSHBACK_BUFFER_SIZE < 18
#error "SERVER_PUSHBACK_BUFFER_SIZE must be at least 18"
#endif

#ifndef SERVER_OUTPUT_BUFFER_SIZE
#if defined(LOW_MEMORY_MCU)
#define SERVER_OUTPUT_BUFFER_SIZE 32
#else
#define SERVER_OUTPUT_BUFFER_SIZE 1024
#endif
#endif

#if SERVER_OUTPUT_BUFFER_SIZE <= 0xF
#define CHUNK_HEADER_SIZE 3
#elif SERVER_OUTPUT_BUFFER_SIZE <= 0xFF
#define CHUNK_HEADER_SIZE 4
#elif SERVER_OUTPUT_BUFFER_SIZE <= 0xFFF
#define CHUNK_HEADER_SIZE 5
#elif SERVER_OUTPUT_BUFFER_SIZE <= 0xFFFF
#define CHUNK_HEADER_SIZE 6
#else
#define CHUNK_HEADER_SIZE 10
#endif

#define CHUNK_TRAILER_SIZE 2
#define CHUNK_OVERHEAD (CHUNK_HEADER_SIZE + CHUNK_TRAILER_SIZE)
#define CHUNK_DATA_SIZE (SERVER_OUTPUT_BUFFER_SIZE - CHUNK_OVERHEAD)

#ifndef SERVER_MAX_HEADERS
#define SERVER_MAX_HEADERS 10
#endif

#ifdef __AVR__
#define P(name)                         \
  static const unsigned char name[]     \
      __attribute__((section(".progmem" \
                             "." #name)))
#else
#define P(name) static const unsigned char name[] PROGMEM
#endif

namespace awot {

class Response;
class Request;
class Router;
class App;

}

#endif
