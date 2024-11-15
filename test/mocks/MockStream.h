#pragma once

#include <Stream.h>

class MockStream : public Stream {
 private:
  const char *readBuffer;
  uint16_t readBufferLength;
  uint16_t readPosition;

  char writeBuffer[1000] = {};
  uint16_t writePosition;

 public:
  MockStream(const char *readBuffer, int bufferLength = -1)
      : readBuffer(readBuffer),
        readBufferLength(bufferLength == -1 ? strlen(readBuffer) : bufferLength),
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
    if (readPosition >= readBufferLength) {
      return -1;
    }

    return readBuffer[readPosition++];
  };

  int available() { return readBufferLength - readPosition; };

  void flush(){};

  char *response() { return writeBuffer; }
};
