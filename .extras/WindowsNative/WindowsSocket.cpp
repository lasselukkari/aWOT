/*
  WindowsSocket.cpp - native Winsock implementation
*/

#include "WindowsSocket.h"

#include <chrono>
#include <climits>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <utility>

// Every live socket retains Winsock initialization, including clients that
// outlive their listener or move into a worker thread.
class WindowsSocketRuntime {
 private:
  bool _ready;

 public:
  WindowsSocketRuntime() : _ready(false) {
    WSADATA data;
    const int error = WSAStartup(MAKEWORD(2, 2), &data);
    if (error != 0) {
      std::fprintf(stderr, "Error starting Winsock: %d\n", error);
      return;
    }
    if (data.wVersion != MAKEWORD(2, 2)) {
      std::fprintf(stderr, "Winsock 2.2 is required\n");
      WSACleanup();
      return;
    }
    _ready = true;
  }

  ~WindowsSocketRuntime() {
    if (_ready) WSACleanup();
  }

  bool ready() const { return _ready; }
};

namespace {

using Clock = std::chrono::steady_clock;

std::shared_ptr<WindowsSocketRuntime> socketRuntime() {
  static std::mutex mutex;
  static std::weak_ptr<WindowsSocketRuntime> shared;
  std::lock_guard<std::mutex> lock(mutex);
  std::shared_ptr<WindowsSocketRuntime> runtime = shared.lock();
  if (!runtime) {
    runtime = std::make_shared<WindowsSocketRuntime>();
    if (!runtime->ready()) return {};
    shared = runtime;
  }
  return runtime;
}

bool setNonblocking(SOCKET socket) {
  u_long enabled = 1;
  return ioctlsocket(socket, FIONBIO, &enabled) == 0;
}

void reportSocketError(const char* message) {
  std::fprintf(stderr, "%s: Winsock error %d\n", message, WSAGetLastError());
}

// Retry interrupted waits without extending the original deadline. Winsock
// ignores select's first argument; SOCKET values must never be narrowed to int.
int waitSocket(SOCKET socket, bool writing, bool indefinite,
               Clock::time_point deadline) {
  for (;;) {
    fd_set descriptors;
    FD_ZERO(&descriptors);
    FD_SET(socket, &descriptors);
    timeval timeout;
    timeval* timeoutPtr = nullptr;
    if (!indefinite) {
      auto remaining = std::chrono::duration_cast<std::chrono::microseconds>(
                           deadline - Clock::now())
                           .count();
      if (remaining < 0) remaining = 0;
      timeout.tv_sec = static_cast<long>(remaining / 1000000);
      timeout.tv_usec = static_cast<long>(remaining % 1000000);
      timeoutPtr = &timeout;
    }
    const int result =
        select(0, writing ? nullptr : &descriptors,
               writing ? &descriptors : nullptr, nullptr, timeoutPtr);
    if (result != SOCKET_ERROR || WSAGetLastError() != WSAEINTR) return result;
    if (!indefinite && Clock::now() >= deadline) return 0;
  }
}

int transferLength(size_t size) {
  return static_cast<int>(size > static_cast<size_t>(INT_MAX) ? INT_MAX : size);
}

}  // namespace

struct WindowsClient::Connection {
  Connection(SOCKET socket, std::shared_ptr<WindowsSocketRuntime> runtime)
      : socket(socket),
        connected(socket != INVALID_SOCKET),
        runtime(std::move(runtime)) {}
  ~Connection() { stop(); }

  void stop() {
    if (socket != INVALID_SOCKET) {
      closesocket(socket);
      socket = INVALID_SOCKET;
    }
    connected = false;
    receiveOffset = receiveLength = 0;
    runtime.reset();
  }

  SOCKET socket;
  bool connected;
  std::shared_ptr<WindowsSocketRuntime> runtime;
  uint8_t receiveBuffer[1024];
  size_t receiveOffset = 0;
  size_t receiveLength = 0;
};

WindowsClient::WindowsClient(SOCKET socket) {
  if (socket == INVALID_SOCKET) return;
  _connection = std::make_shared<Connection>(socket, socketRuntime());
  if (!_connection->runtime || !setNonblocking(_connection->socket)) stop();
}

int WindowsClient::connect(IPAddress ip, uint16_t port) {
  stop();
  if (ip.type() != arduino::IPv4) return 0;
  auto runtime = socketRuntime();
  if (!runtime) return 0;
  _connection =
      std::make_shared<Connection>(INVALID_SOCKET, std::move(runtime));
  _connection->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (_connection->socket == INVALID_SOCKET) {
    stop();
    return 0;
  }

  sockaddr_in address = {};
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  // IPAddress already stores its octets in network byte order.
  address.sin_addr.s_addr = static_cast<uint32_t>(ip);
  if (::connect(_connection->socket, reinterpret_cast<sockaddr*>(&address),
                sizeof(address)) == SOCKET_ERROR ||
      !setNonblocking(_connection->socket)) {
    stop();
    return 0;
  }
  _connection->connected = true;
  return 1;
}

int WindowsClient::connect(const char* host, uint16_t port) {
  // Numeric IPv4 addresses only; no DNS lookup or IPv6 connection.
  in_addr address = {};
  if (host && InetPtonA(AF_INET, host, &address) == 1) {
    return connect(IPAddress(static_cast<uint32_t>(address.s_addr)), port);
  }
  stop();
  return 0;
}

size_t WindowsClient::write(uint8_t byte) { return write(&byte, 1); }

size_t WindowsClient::write(const uint8_t* buf, size_t size) {
  if (!_connection || _connection->socket == INVALID_SOCKET ||
      !_connection->connected || size == 0)
    return 0;
  const auto deadline = Clock::now() + std::chrono::milliseconds(getTimeout());
  for (;;) {
    const int result =
        send(_connection->socket, reinterpret_cast<const char*>(buf),
             transferLength(size), 0);
    // aWOT retries partial writes using the returned count.
    if (result >= 0) return static_cast<size_t>(result);
    const int error = WSAGetLastError();
    if (error == WSAEINTR) {
      if (Clock::now() >= deadline) return 0;
      continue;
    }
    if (error != WSAEWOULDBLOCK) {
      _connection->connected = false;
      return 0;
    }
    // aWOT treats zero as a failed write. Give temporary backpressure a
    // bounded chance to clear, without blocking receive or Stream timeouts.
    const int ready = waitSocket(_connection->socket, true, false, deadline);
    if (ready <= 0) {
      if (ready == SOCKET_ERROR) _connection->connected = false;
      return 0;
    }
    if (Clock::now() >= deadline) return 0;
  }
}

int WindowsClient::receive(uint8_t* buf, size_t size) {
  if (!_connection || _connection->socket == INVALID_SOCKET ||
      !_connection->connected)
    return -1;
  if (size == 0) return 0;
  // Sockets are nonblocking, leaving all read timeouts to Arduino Stream.
  int result;
  do {
    result = recv(_connection->socket, reinterpret_cast<char*>(buf),
                  transferLength(size), 0);
  } while (result == SOCKET_ERROR && WSAGetLastError() == WSAEINTR);
  if (result == 0 ||
      (result == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)) {
    _connection->connected = false;
  }
  return result;
}

bool WindowsClient::fillReceiveBuffer() {
  if (!_connection) return false;
  if (_connection->receiveOffset < _connection->receiveLength) return true;
  _connection->receiveOffset = _connection->receiveLength = 0;
  const int received =
      receive(_connection->receiveBuffer, sizeof(_connection->receiveBuffer));
  if (received > 0) _connection->receiveLength = static_cast<size_t>(received);
  return _connection->receiveLength > 0;
}

int WindowsClient::available() {
  if (!_connection) return 0;
  fillReceiveBuffer();
  return static_cast<int>(_connection->receiveLength -
                          _connection->receiveOffset);
}

int WindowsClient::read() {
  return fillReceiveBuffer()
             ? _connection->receiveBuffer[_connection->receiveOffset++]
             : -1;
}

int WindowsClient::read(uint8_t* buf, size_t size) {
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
  const int result = receive(buf, size);
  return result > 0 ? result : 0;
}

int WindowsClient::peek() {
  return fillReceiveBuffer()
             ? _connection->receiveBuffer[_connection->receiveOffset]
             : -1;
}

void WindowsClient::flush() {
  // Socket writes are handed to Winsock immediately.
}

void WindowsClient::stop() {
  if (_connection) _connection->stop();
  _connection.reset();
}

uint8_t WindowsClient::connected() {
  const bool buffered = fillReceiveBuffer();
  return (_connection && _connection->connected) || buffered ? 1 : 0;
}

WindowsClient::operator bool() {
  return _connection &&
         (_connection->connected ||
          _connection->receiveOffset < _connection->receiveLength);
}

SOCKET WindowsClient::getSocket() const {
  return _connection ? _connection->socket : INVALID_SOCKET;
}

bool WindowsServer::begin() {
  stop();
  _runtime = socketRuntime();
  if (!_runtime) return false;
  _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (_socket == INVALID_SOCKET) {
    reportSocketError("Error creating listener");
    stop();
    return false;
  }

  // On Windows SO_REUSEADDR could let a second server hijack this port.
  const BOOL exclusive = TRUE;
  if (setsockopt(_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                 reinterpret_cast<const char*>(&exclusive),
                 sizeof(exclusive)) == SOCKET_ERROR) {
    reportSocketError("Error configuring listener");
    stop();
    return false;
  }

  sockaddr_in address = {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(_port);
  if (bind(_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) ==
      SOCKET_ERROR) {
    reportSocketError("Error binding listener");
    stop();
    return false;
  }
  if (listen(_socket, 5) == SOCKET_ERROR || !setNonblocking(_socket)) {
    reportSocketError("Error starting listener");
    stop();
    return false;
  }

  // Report the selected port for an ephemeral listener.
  int addressLength = sizeof(address);
  if (getsockname(_socket, reinterpret_cast<sockaddr*>(&address),
                  &addressLength) == SOCKET_ERROR) {
    reportSocketError("Error reading listener address");
    stop();
    return false;
  }
  _port = ntohs(address.sin_port);
  return true;
}

void WindowsServer::stop() {
  if (_socket != INVALID_SOCKET) {
    closesocket(_socket);
    _socket = INVALID_SOCKET;
  }
  _runtime.reset();
}

WindowsClient WindowsServer::available(int timeoutMs) {
  if (_socket == INVALID_SOCKET) return WindowsClient();
  const auto deadline =
      Clock::now() + std::chrono::milliseconds(timeoutMs < 0 ? 0 : timeoutMs);
  if (waitSocket(_socket, false, timeoutMs < 0, deadline) <= 0) {
    return WindowsClient();
  }

  SOCKET accepted;
  do {
    accepted = accept(_socket, nullptr, nullptr);
  } while (accepted == INVALID_SOCKET && WSAGetLastError() == WSAEINTR);
  return WindowsClient(accepted);
}
