/*
  MacOSSocket.h - BSD socket wrapper for macOS with Arduino Client/Server API
*/

#ifndef MACOSSOCKET_H_
#define MACOSSOCKET_H_

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

// Undef system macro to avoid conflict with arduino::INADDR_NONE
#ifdef INADDR_NONE
#undef INADDR_NONE
#endif

#include "Arduino.h"
#include "Client.h"

class MacOSClient : public Client {
 private:
  int _socket;
  bool _connected;

 public:
  MacOSClient() : _socket(-1), _connected(false) {}
  MacOSClient(int socket) : _socket(socket), _connected(socket >= 0) {}

  // Move constructor - transfer ownership
  MacOSClient(MacOSClient&& other) noexcept
      : _socket(other._socket), _connected(other._connected) {
    other._socket = -1;
    other._connected = false;
  }

  // Move assignment
  MacOSClient& operator=(MacOSClient&& other) noexcept {
    if (this != &other) {
      if (_socket >= 0) close(_socket);
      _socket = other._socket;
      _connected = other._connected;
      other._socket = -1;
      other._connected = false;
    }
    return *this;
  }

  // Disable copy (socket can't be shared)
  MacOSClient(const MacOSClient&) = delete;
  MacOSClient& operator=(const MacOSClient&) = delete;

  // Client interface implementation
  int connect(IPAddress ip, uint16_t port) override;
  int connect(const char* host, uint16_t port) override;
  size_t write(uint8_t byte) override;
  size_t write(const uint8_t* buf, size_t size) override;
  int available() override;
  int read() override;
  int read(uint8_t* buf, size_t size) override;
  int peek() override;
  void flush() override;
  void stop() override;
  uint8_t connected() override;
  operator bool() override { return _connected; }

  // Get the underlying socket (for debugging)
  int getSocket() const { return _socket; }
};

class MacOSServer {
 private:
  uint16_t _port;
  int _socket;

 public:
  MacOSServer(uint16_t port) : _port(port), _socket(-1) {}

  void begin();
  MacOSClient available();

  uint16_t port() const { return _port; }
};

#endif
