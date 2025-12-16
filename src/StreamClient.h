/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things
*/

#ifndef AWOT_STREAM_CLIENT_H_
#define AWOT_STREAM_CLIENT_H_

#include "aWOT_common.h"

namespace awot {

class StreamClient : public Client {
 private:
  Stream* s;

 public:
  StreamClient(Stream* stream) : s(stream) {};
  int connect(IPAddress, uint16_t) { return 1; };
  int connect(const char*, uint16_t) { return 1; };
  size_t write(uint8_t byte) { return s->write(byte); };
  size_t write(const uint8_t* buffer, size_t length) {
    return s->write(buffer, length);
  };
  int available() { return s->available(); };
  int read() { return s->read(); };
  int read(uint8_t* buffer, size_t length) {
    size_t count = 0;

    while (count < length) {
      int c = read();
      if (c < 0) {
        break;
      }

      *buffer++ = (uint8_t)c;
      count++;
    }

    return count;
  }
  int peek() { return s->peek(); };
  void flush() { return s->flush(); };
  void stop() {};
  uint8_t connected() { return 1; };
  operator bool() { return true; };
};

}  // namespace awot

#endif
