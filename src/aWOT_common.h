/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef AWOT_COMMON_H_
#define AWOT_COMMON_H_

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#include "Client.h"

#if defined(STD_FUNCTION_MIDDLEWARE) || defined(ESP8266) || defined(ESP32) || \
    (defined(__has_include) && __has_include(<functional>))
#ifndef STD_FUNCTION_MIDDLEWARE
#define STD_FUNCTION_MIDDLEWARE
#endif
#include <functional>
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

#ifndef SERVER_OUTPUT_BUFFER_SIZE
#if defined(LOW_MEMORY_MCU)
#define SERVER_OUTPUT_BUFFER_SIZE 32
#else
#define SERVER_OUTPUT_BUFFER_SIZE 1024
#endif
#endif

// Calculate chunk header size based on buffer size
// Header format: "<hex_size>\r\n" + data + "\r\n"
#if SERVER_OUTPUT_BUFFER_SIZE <= 0xF
#define CHUNK_HEADER_SIZE 3  // "F\r\n"
#elif SERVER_OUTPUT_BUFFER_SIZE <= 0xFF
#define CHUNK_HEADER_SIZE 4  // "FF\r\n"
#elif SERVER_OUTPUT_BUFFER_SIZE <= 0xFFF
#define CHUNK_HEADER_SIZE 5  // "FFF\r\n"
#elif SERVER_OUTPUT_BUFFER_SIZE <= 0xFFFF
#define CHUNK_HEADER_SIZE 6  // "FFFF\r\n"
#else
#define CHUNK_HEADER_SIZE 10  // "FFFFFFFF\r\n"
#endif

#define CHUNK_TRAILER_SIZE 2  // "\r\n"
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

// Forward declarations
class Response;
class Request;
class Router;
class App;

}  // namespace awot

#endif
