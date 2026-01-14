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

#ifndef AWOT_WEBSOCKET_COMMON_H_
#define AWOT_WEBSOCKET_COMMON_H_

#include <Arduino.h>
#include <stdint.h>

#include "Client.h"

#ifndef WEBSOCKET_MAX_CLIENTS
#if defined(LOW_MEMORY_MCU)
#define WEBSOCKET_MAX_CLIENTS 2
#else
#define WEBSOCKET_MAX_CLIENTS 4
#endif
#endif

#ifndef WEBSOCKET_BUFFER_SIZE
#if defined(LOW_MEMORY_MCU)
#define WEBSOCKET_BUFFER_SIZE 128
#else
#define WEBSOCKET_BUFFER_SIZE 256
#endif
#endif

#ifndef WEBSOCKET_MAX_ROOMS
#define WEBSOCKET_MAX_ROOMS 8
#endif

namespace awot {

// WebSocket opcodes (RFC 6455)
enum WebSocketOpcode {
  WS_OPCODE_CONTINUATION = 0x00,
  WS_OPCODE_TEXT = 0x01,
  WS_OPCODE_BINARY = 0x02,
  WS_OPCODE_CLOSE = 0x08,
  WS_OPCODE_PING = 0x09,
  WS_OPCODE_PONG = 0x0A
};

// Forward declaration
class WebSocket;

}  // namespace awot

#endif
