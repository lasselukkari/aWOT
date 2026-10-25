/*
  MacOSSocket.h - BSD socket wrapper for macOS with Arduino Client/Server API
*/

#ifndef MACOSSOCKET_H_
#define MACOSSOCKET_H_

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <memory>

// Undef system macro to avoid conflict with arduino::INADDR_NONE
#ifdef INADDR_NONE
#undef INADDR_NONE
#endif

#include "Arduino.h"
#include "Client.h"

class MacOSClient : public Client {
 private:
  struct Connection;
  std::shared_ptr<Connection> _connection;
  ssize_t receive(uint8_t* buf, size_t size, int flags);
  bool fillReceiveBuffer();

 public:
  MacOSClient() = default;
  explicit MacOSClient(int socket);
  // Copies share the socket and unread bytes. Only the last handle's
  // destruction closes the connection; stop() closes it for every copy.
  ~MacOSClient() = default;
  MacOSClient(const MacOSClient&) = default;
  MacOSClient& operator=(const MacOSClient&) = default;
  MacOSClient(MacOSClient&&) noexcept = default;
  MacOSClient& operator=(MacOSClient&&) noexcept = default;

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

  int getSocket() const;
};

class MacOSServer {
 private:
  uint16_t _port;
  int _socket;

 public:
  MacOSServer(uint16_t port) : _port(port), _socket(-1) {}
  ~MacOSServer() { stop(); }

  MacOSServer(const MacOSServer&) = delete;
  MacOSServer& operator=(const MacOSServer&) = delete;

  bool begin();
  void stop();
  // A zero timeout polls; -1 waits indefinitely for a connection.
  MacOSClient available(int timeoutMs = 0);

  uint16_t port() const { return _port; }
};

#endif
