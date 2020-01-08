#pragma once

#include <Stream.h>

class MockStream : public Stream {
 private:
  char const *readBuffer;
  uint16_t readBufferLenght;
  uint16_t readPosition;

  char writeBuffer[1000] = {};
  uint16_t writePosition;

 public:
  MockStream(const char *readBuffer)
      : readBuffer(readBuffer),
        readBufferLenght(strlen(readBuffer)),
        readPosition(0),
        writePosition(0){};

  ~MockStream() {}

  size_t write(uint8_t byte) {
    if (writePosition >= 1000) {
      return 0;
    }

    writeBuffer[writePosition++] = byte;

    return 1;
  };

  int peek() { return readBuffer[readPosition]; };

  int read() {
    if (readPosition >= readBufferLenght) {
      return -1;
    }

    return readBuffer[readPosition++];
  };

  int available() { return readPosition - readBufferLenght; };

  void flush(){};

  char *response() { return writeBuffer; }
};
