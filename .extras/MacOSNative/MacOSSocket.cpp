/*
  MacOSSocket.cpp - BSD socket implementation for macOS
*/

#include "MacOSSocket.h"

#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

SerialClass Serial;

size_t SerialClass::write(uint8_t c) {
  std::cout << (char)c;
  return 1;
}

size_t SerialClass::write(const uint8_t* buffer, size_t size) {
  std::cout.write((const char*)buffer, size);
  return size;
}

void SerialClass::flush() { std::cout.flush(); }

namespace {

std::chrono::steady_clock::duration elapsedTime() {
  static const auto start = std::chrono::steady_clock::now();
  return std::chrono::steady_clock::now() - start;
}

template <typename Unsigned>
char* formatUnsigned(Unsigned value, char* buffer, int base) {
  if (base < 2 || base > 36) {
    buffer[0] = '\0';
    return buffer;
  }

  static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  char* end = buffer;
  do {
    *end++ = digits[value % base];
    value /= base;
  } while (value);
  *end = '\0';

  for (char *left = buffer, *right = end - 1; left < right; ++left, --right) {
    const char digit = *left;
    *left = *right;
    *right = digit;
  }
  return buffer;
}

}  // namespace

extern "C" {

unsigned long millis(void) {
  return static_cast<unsigned long>(
      std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime())
          .count());
}

unsigned long micros(void) {
  return static_cast<unsigned long>(
      std::chrono::duration_cast<std::chrono::microseconds>(elapsedTime())
          .count());
}

void delay(unsigned long ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void yield(void) { std::this_thread::yield(); }

char* itoa(int value, char* buffer, int base) {
  const unsigned magnitude = static_cast<unsigned>(value);
  if (base == 10 && value < 0) {
    buffer[0] = '-';
    formatUnsigned(0U - magnitude, buffer + 1, base);
    return buffer;
  }
  return formatUnsigned(magnitude, buffer, base);
}

char* ltoa(long value, char* buffer, int base) {
  const unsigned long magnitude = static_cast<unsigned long>(value);
  if (base == 10 && value < 0) {
    buffer[0] = '-';
    formatUnsigned(0UL - magnitude, buffer + 1, base);
    return buffer;
  }
  return formatUnsigned(magnitude, buffer, base);
}

char* utoa(unsigned value, char* buffer, int base) {
  return formatUnsigned(value, buffer, base);
}

char* ultoa(unsigned long value, char* buffer, int base) {
  return formatUnsigned(value, buffer, base);
}

char* dtostrf(double value, signed char width, unsigned char precision,
              char* buffer) {
  const int length = snprintf(nullptr, 0, "%*.*f", width, precision, value);
  if (length < 0) {
    buffer[0] = '\0';
    return buffer;
  }
  snprintf(buffer, static_cast<size_t>(length) + 1, "%*.*f", width, precision,
           value);
  return buffer;
}

}  // extern "C"

namespace {

bool suppressSigpipe(int socket) {
#ifdef SO_NOSIGPIPE
  int enabled = 1;
  return setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, &enabled,
                    sizeof(enabled)) == 0;
#elif defined(MSG_NOSIGNAL)
  // Linux suppresses SIGPIPE on each send instead of through a socket option.
  (void)socket;
  return true;
#else
#error "This POSIX platform needs a way to suppress socket SIGPIPE"
#endif
}

bool wouldBlock() { return errno == EAGAIN || errno == EWOULDBLOCK; }

}  // namespace

struct MacOSClient::Connection {
  explicit Connection(int socket) : socket(socket), connected(socket >= 0) {}
  ~Connection() { stop(); }

  void stop() {
    if (socket >= 0) {
      close(socket);
      socket = -1;
    }
    connected = false;
    receiveOffset = receiveLength = 0;
  }

  int socket;
  bool connected;
  uint8_t receiveBuffer[1024];
  size_t receiveOffset = 0;
  size_t receiveLength = 0;
};

MacOSClient::MacOSClient(int socket) {
  if (socket < 0) return;
  _connection = std::make_shared<Connection>(socket);
  if (!suppressSigpipe(socket)) stop();
}

int MacOSClient::connect(IPAddress ip, uint16_t port) {
  stop();
  if (ip.type() != arduino::IPv4) return 0;

  _connection = std::make_shared<Connection>(-1);
  _connection->socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_connection->socket < 0 || !suppressSigpipe(_connection->socket)) {
    stop();
    return 0;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  // IPAddress already stores the octets in network byte order.
  addr.sin_addr.s_addr = (uint32_t)ip;

  if (::connect(_connection->socket, (struct sockaddr*)&addr, sizeof(addr)) <
      0) {
    stop();
    return 0;
  }

  _connection->connected = true;
  return 1;
}

int MacOSClient::connect(const char* host, uint16_t port) {
  // Only numeric IPv4 addresses are supported; DNS is not resolved.
  struct in_addr addr;
  if (host && inet_aton(host, &addr)) {
    return connect(IPAddress((uint32_t)addr.s_addr), port);
  }
  stop();
  return 0;
}

size_t MacOSClient::write(uint8_t byte) { return write(&byte, 1); }

size_t MacOSClient::write(const uint8_t* buf, size_t size) {
  if (!_connection || _connection->socket < 0 || !_connection->connected)
    return 0;

  ssize_t result;
  do {
#ifdef MSG_NOSIGNAL
    result = send(_connection->socket, buf, size, MSG_NOSIGNAL);
#else
    result = send(_connection->socket, buf, size, 0);
#endif
  } while (result < 0 && errno == EINTR);
  if (result < 0) {
    if (!wouldBlock()) _connection->connected = false;
    return 0;
  }
  return (size_t)result;
}

ssize_t MacOSClient::receive(uint8_t* buf, size_t size, int flags) {
  if (!_connection || _connection->socket < 0 || !_connection->connected)
    return -1;
  if (size == 0) return 0;

  // Stream implements its own timeouts, so an empty read must return promptly.
  ssize_t result;
  do {
    result = recv(_connection->socket, buf, size, flags | MSG_DONTWAIT);
  } while (result < 0 && errno == EINTR);
  if (result == 0 || (result < 0 && !wouldBlock())) {
    _connection->connected = false;
  }
  return result;
}

bool MacOSClient::fillReceiveBuffer() {
  if (!_connection) return false;
  if (_connection->receiveOffset < _connection->receiveLength) return true;
  _connection->receiveOffset = _connection->receiveLength = 0;
  const ssize_t received = receive(_connection->receiveBuffer,
                                   sizeof(_connection->receiveBuffer), 0);
  if (received > 0) _connection->receiveLength = static_cast<size_t>(received);
  return _connection->receiveLength > 0;
}

int MacOSClient::available() {
  if (!_connection) return 0;
  fillReceiveBuffer();
  return static_cast<int>(_connection->receiveLength -
                          _connection->receiveOffset);
}

int MacOSClient::read() {
  return fillReceiveBuffer()
             ? _connection->receiveBuffer[_connection->receiveOffset++]
             : -1;
}

int MacOSClient::read(uint8_t* buf, size_t size) {
  if (size == 0 || !_connection) return 0;
  if (_connection->receiveOffset < _connection->receiveLength) {
    const size_t buffered =
        _connection->receiveLength - _connection->receiveOffset;
    const size_t count = size < buffered ? size : buffered;
    std::memcpy(buf, _connection->receiveBuffer + _connection->receiveOffset,
                count);
    _connection->receiveOffset += count;
    return static_cast<int>(count);
  }
  ssize_t result = receive(buf, size, 0);
  return result > 0 ? (int)result : 0;
}

int MacOSClient::peek() {
  return fillReceiveBuffer()
             ? _connection->receiveBuffer[_connection->receiveOffset]
             : -1;
}

void MacOSClient::flush() {
  // Nothing to flush for sockets - data is sent immediately
}

void MacOSClient::stop() {
  if (_connection) _connection->stop();
  _connection.reset();
}

uint8_t MacOSClient::connected() {
  const bool buffered = fillReceiveBuffer();
  return (_connection && _connection->connected) || buffered ? 1 : 0;
}

MacOSClient::operator bool() {
  return _connection &&
         (_connection->connected ||
          _connection->receiveOffset < _connection->receiveLength);
}

int MacOSClient::getSocket() const {
  return _connection ? _connection->socket : -1;
}

bool MacOSServer::begin() {
  stop();
  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket < 0) {
    perror("Error creating socket");
    return false;
  }

  // Allow address reuse for quick restarts
  int opt = 1;
  if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("Error configuring socket");
    stop();
    return false;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(_port);

  if (bind(_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("Error binding socket");
    stop();
    return false;
  }

  if (listen(_socket, 5) < 0) {
    perror("Error listening on socket");
    stop();
    return false;
  }

  int flags = fcntl(_socket, F_GETFL, 0);
  if (flags < 0 || fcntl(_socket, F_SETFL, flags | O_NONBLOCK) < 0) {
    perror("Error configuring listener");
    stop();
    return false;
  }

  // Report the actual port when an ephemeral port was requested.
  socklen_t addrLen = sizeof(addr);
  if (getsockname(_socket, (struct sockaddr*)&addr, &addrLen) < 0) {
    perror("Error reading listener address");
    stop();
    return false;
  }
  _port = ntohs(addr.sin_port);
  return true;
}

void MacOSServer::stop() {
  if (_socket >= 0) {
    close(_socket);
    _socket = -1;
  }
}

MacOSClient MacOSServer::available(int timeoutMs) {
  if (_socket < 0) {
    return MacOSClient();
  }

  struct pollfd listener = {_socket, POLLIN, 0};
  int ready;
  do {
    ready = poll(&listener, 1, timeoutMs);
  } while (ready < 0 && errno == EINTR);
  if (ready <= 0 || !(listener.revents & POLLIN)) return MacOSClient();

  int clientSocket;
  do {
    clientSocket = accept(_socket, nullptr, nullptr);
  } while (clientSocket < 0 && errno == EINTR);

  if (clientSocket < 0) {
    return MacOSClient();
  }

  int flags = fcntl(clientSocket, F_GETFL, 0);
  if (flags < 0 || fcntl(clientSocket, F_SETFL, flags & ~O_NONBLOCK) < 0) {
    close(clientSocket);
    return MacOSClient();
  }

  return MacOSClient(clientSocket);
}
