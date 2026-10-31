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

#include <memory>

// Avoid a conflict with arduino::INADDR_NONE in IPAddress.h.
#ifdef INADDR_NONE
#undef INADDR_NONE
#endif

#include "Arduino.h"
#include "Client.h"

class WindowsSocketRuntime;

class WindowsClient : public Client {
 private:
  struct Connection;
  std::shared_ptr<Connection> _connection;

  int receive(uint8_t* buf, size_t size);
  bool fillReceiveBuffer();

 public:
  WindowsClient() = default;
  // Takes ownership of a connected Winsock socket.
  explicit WindowsClient(SOCKET socket);
  // Copies share the socket and unread bytes. Only the last handle's
  // destruction closes the connection; stop() closes it for every copy.
  ~WindowsClient() = default;
  WindowsClient(const WindowsClient&) = default;
  WindowsClient& operator=(const WindowsClient&) = default;
  WindowsClient(WindowsClient&&) noexcept = default;
  WindowsClient& operator=(WindowsClient&&) noexcept = default;

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
  operator bool() override;

  SOCKET getSocket() const;
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
