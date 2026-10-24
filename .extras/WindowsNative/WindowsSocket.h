/*
  WindowsSocket.h - Winsock wrapper with the Arduino Client/Server API
*/

#ifndef WINDOWSSOCKET_H_
#define WINDOWSSOCKET_H_

// Winsock2 must precede headers that can include the older winsock.h.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
// Arduino's global INPUT pin mode conflicts with Win32's UI INPUT typedef.
#ifndef NOUSER
#define NOUSER
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

// The Windows access-rights macro conflicts with Request::MethodType::DELETE.
#ifdef DELETE
#undef DELETE
#endif

#include <cstring>
#include <memory>
#include <utility>

// Avoid a conflict with arduino::INADDR_NONE in IPAddress.h.
#ifdef INADDR_NONE
#undef INADDR_NONE
#endif

#include "Arduino.h"
#include "Client.h"

class WindowsSocketRuntime;

class WindowsClient : public Client {
 private:
  SOCKET _socket = INVALID_SOCKET;
  bool _connected = false;
  std::shared_ptr<WindowsSocketRuntime> _runtime;
  uint8_t _receiveBuffer[1024];
  size_t _receiveOffset = 0;
  size_t _receiveLength = 0;

  int receive(uint8_t* buf, size_t size);
  bool fillReceiveBuffer();
  void moveReceiveBuffer(WindowsClient& other) {
    _receiveOffset = 0;
    _receiveLength = other._receiveLength - other._receiveOffset;
    std::memcpy(_receiveBuffer, other._receiveBuffer + other._receiveOffset,
                _receiveLength);
    other._receiveOffset = other._receiveLength = 0;
  }

 public:
  WindowsClient() = default;
  // Takes ownership of a connected Winsock socket.
  explicit WindowsClient(SOCKET socket);
  ~WindowsClient() { stop(); }

  WindowsClient(WindowsClient&& other) noexcept
      : _socket(other._socket),
        _connected(other._connected),
        _runtime(std::move(other._runtime)) {
    moveReceiveBuffer(other);
    other._socket = INVALID_SOCKET;
    other._connected = false;
  }

  WindowsClient& operator=(WindowsClient&& other) noexcept {
    if (this != &other) {
      stop();
      _socket = other._socket;
      _connected = other._connected;
      _runtime = std::move(other._runtime);
      moveReceiveBuffer(other);
      other._socket = INVALID_SOCKET;
      other._connected = false;
    }
    return *this;
  }

  WindowsClient(const WindowsClient&) = delete;
  WindowsClient& operator=(const WindowsClient&) = delete;

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
  operator bool() override {
    return _connected || _receiveOffset < _receiveLength;
  }

  SOCKET getSocket() const { return _socket; }
};

class WindowsServer {
 private:
  uint16_t _port;
  SOCKET _socket = INVALID_SOCKET;
  std::shared_ptr<WindowsSocketRuntime> _runtime;

 public:
  explicit WindowsServer(uint16_t port) : _port(port) {}
  ~WindowsServer() { stop(); }
  WindowsServer(const WindowsServer&) = delete;
  WindowsServer& operator=(const WindowsServer&) = delete;

  bool begin();
  void stop();
  // A zero timeout polls; -1 waits indefinitely for a connection.
  WindowsClient available(int timeoutMs = 0);
  uint16_t port() const { return _port; }
};

#endif
