#pragma once

#include <Client.h>

#include <cstring>

class MockClient : public Client {
 private:
  uint8_t* m_readBuffer;
  size_t m_readBufferLength;
  size_t m_readPosition;

  uint8_t m_writeBuffer[2048];
  size_t m_writePosition;

  bool m_connected;

 public:
  MockClient()
      : m_readBuffer(nullptr),
        m_readBufferLength(0),
        m_readPosition(0),
        m_writePosition(0),
        m_connected(true) {}

  MockClient(const uint8_t* readBuffer, size_t bufferLength)
      : m_readBuffer(new uint8_t[bufferLength]),
        m_readBufferLength(bufferLength),
        m_readPosition(0),
        m_writePosition(0),
        m_connected(true) {
    memcpy(m_readBuffer, readBuffer, bufferLength);
  }

  ~MockClient() { delete[] m_readBuffer; }

  void setReadBuffer(const uint8_t* buffer, size_t length) {
    delete[] m_readBuffer;
    m_readBuffer = new uint8_t[length];
    memcpy(m_readBuffer, buffer, length);
    m_readBufferLength = length;
    m_readPosition = 0;
  }

  void setConnected(bool connected) { m_connected = connected; }

  int connect(IPAddress ip, uint16_t port) override {
    (void)ip;
    (void)port;
    m_connected = true;
    return 1;
  }

  int connect(const char* host, uint16_t port) override {
    (void)host;
    (void)port;
    m_connected = true;
    return 1;
  }

  size_t write(uint8_t byte) override {
    if (m_writePosition >= sizeof(m_writeBuffer)) {
      return 0;
    }
    m_writeBuffer[m_writePosition++] = byte;
    return 1;
  }

  size_t write(const uint8_t* buf, size_t size) override {
    size_t written = 0;
    for (size_t i = 0; i < size && m_writePosition < sizeof(m_writeBuffer);
         i++) {
      m_writeBuffer[m_writePosition++] = buf[i];
      written++;
    }
    return written;
  }

  int available() override { return m_readBufferLength - m_readPosition; }

  int read() override {
    if (m_readPosition >= m_readBufferLength) {
      return -1;
    }
    return m_readBuffer[m_readPosition++];
  }

  int read(uint8_t* buf, size_t size) override {
    size_t bytesRead = 0;
    while (bytesRead < size && m_readPosition < m_readBufferLength) {
      buf[bytesRead++] = m_readBuffer[m_readPosition++];
    }
    return bytesRead;
  }

  int peek() override {
    if (m_readPosition >= m_readBufferLength) {
      return -1;
    }
    return m_readBuffer[m_readPosition];
  }

  void flush() override {}

  void stop() override { m_connected = false; }

  uint8_t connected() override { return m_connected ? 1 : 0; }

  operator bool() override { return m_connected; }

  const uint8_t* getWriteBuffer() const { return m_writeBuffer; }
  size_t getWriteLength() const { return m_writePosition; }

  const char* getWriteBufferAsString() const {
    return reinterpret_cast<const char*>(m_writeBuffer);
  }

  void clearWriteBuffer() { m_writePosition = 0; }

  bool writeBufferContains(const char* str) const {
    return strstr(getWriteBufferAsString(), str) != nullptr;
  }
};
