/*
  MacOSSocket.cpp - BSD socket implementation for macOS
*/

#include "MacOSSocket.h"

#include <iostream>

// ============================================================================
// SerialClass implementation (from Arduino.h)
// ============================================================================

SerialClass Serial;

size_t SerialClass::write(uint8_t c) {
  std::cout << (char)c;
  return 1;
}

size_t SerialClass::write(const uint8_t* buffer, size_t size) {
  std::cout.write((const char*)buffer, size);
  return size;
}

// ============================================================================
// Platform function implementations (extern "C" from Arduino.h)
// ============================================================================

extern "C" {

unsigned long millis(void) {
  static unsigned long start = 0;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned long now = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
  if (start == 0) start = now;
  return now - start;
}

unsigned long micros(void) {
  static unsigned long start = 0;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned long now = (tv.tv_sec * 1000000) + tv.tv_usec;
  if (start == 0) start = now;
  return now - start;
}

void delay(unsigned long ms) {
  unsigned long start_time = millis();
  while (millis() - start_time < ms);
}

void yield(void) {
  // No-op on macOS
}

char* itoa(int value, char* buffer, int base) {
  if (base == 10) {
    snprintf(buffer, 32, "%d", value);
  } else if (base == 16) {
    snprintf(buffer, 32, "%x", (unsigned)value);
  } else if (base == 8) {
    snprintf(buffer, 32, "%o", (unsigned)value);
  } else {
    snprintf(buffer, 32, "%d", value);
  }
  return buffer;
}

char* ltoa(long value, char* buffer, int base) {
  if (base == 10) {
    snprintf(buffer, 32, "%ld", value);
  } else if (base == 16) {
    snprintf(buffer, 32, "%lx", (unsigned long)value);
  } else {
    snprintf(buffer, 32, "%ld", value);
  }
  return buffer;
}

char* utoa(unsigned value, char* buffer, int base) {
  if (base == 10) {
    snprintf(buffer, 32, "%u", value);
  } else if (base == 16) {
    snprintf(buffer, 32, "%x", value);
  } else {
    snprintf(buffer, 32, "%u", value);
  }
  return buffer;
}

char* ultoa(unsigned long value, char* buffer, int base) {
  if (base == 10) {
    snprintf(buffer, 32, "%lu", value);
  } else if (base == 16) {
    snprintf(buffer, 32, "%lx", value);
  } else {
    snprintf(buffer, 32, "%lu", value);
  }
  return buffer;
}

char* dtostrf(double value, signed char width, unsigned char precision,
              char* buffer) {
  snprintf(buffer, 32, "%*.*f", width, precision, value);
  return buffer;
}

}  // extern "C"

// ============================================================================
// MacOSClient implementation
// ============================================================================

int MacOSClient::connect(IPAddress ip, uint16_t port) {
  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket < 0) {
    return 0;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl((uint32_t)ip);

  if (::connect(_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    close(_socket);
    _socket = -1;
    return 0;
  }

  _connected = true;
  return 1;
}

int MacOSClient::connect(const char* host, uint16_t port) {
  // Simple implementation - just try to parse as IP address
  // For a full implementation, you'd use getaddrinfo()
  struct in_addr addr;
  if (inet_aton(host, &addr)) {
    return connect(IPAddress((uint32_t)ntohl(addr.s_addr)), port);
  }
  return 0;
}

size_t MacOSClient::write(uint8_t byte) { return write(&byte, 1); }

size_t MacOSClient::write(const uint8_t* buf, size_t size) {
  if (_socket < 0 || !_connected) return 0;

  ssize_t result = send(_socket, buf, size, 0);
  if (result < 0) {
    _connected = false;
    return 0;
  }
  return (size_t)result;
}

int MacOSClient::available() {
  if (_socket < 0 || !_connected) return 0;

  // Use select with zero timeout to check if data is available
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(_socket, &readfds);

  struct timeval tv = {0, 0};
  int result = select(_socket + 1, &readfds, nullptr, nullptr, &tv);

  if (result > 0 && FD_ISSET(_socket, &readfds)) {
    // Peek to see how much data is available
    uint8_t buf[1];
    result = recv(_socket, buf, 1, MSG_PEEK | MSG_DONTWAIT);
    if (result > 0) return 1;  // At least 1 byte available
    if (result == 0) {
      // Connection closed
      _connected = false;
    }
  }
  return 0;
}

int MacOSClient::read() {
  if (_socket < 0 || !_connected) return -1;

  uint8_t b;
  ssize_t result = recv(_socket, &b, 1, 0);
  if (result <= 0) {
    if (result == 0) _connected = false;
    return -1;
  }
  return b;
}

int MacOSClient::read(uint8_t* buf, size_t size) {
  if (_socket < 0 || !_connected) return 0;

  ssize_t result = recv(_socket, buf, size, 0);
  if (result < 0) return 0;
  if (result == 0) _connected = false;
  return (int)result;
}

int MacOSClient::peek() {
  if (_socket < 0 || !_connected) return -1;

  uint8_t b;
  ssize_t result = recv(_socket, &b, 1, MSG_PEEK);
  if (result <= 0) {
    if (result == 0) _connected = false;
    return -1;
  }
  return b;
}

void MacOSClient::flush() {
  // Nothing to flush for sockets - data is sent immediately
}

void MacOSClient::stop() {
  if (_socket >= 0) {
    close(_socket);
    _socket = -1;
  }
  _connected = false;
}

uint8_t MacOSClient::connected() {
  if (_socket < 0) return 0;

  // Check if still connected by peeking
  if (_connected) {
    uint8_t buf;
    int result = recv(_socket, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
    if (result == 0) {
      // Connection closed by peer
      _connected = false;
    } else if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
      // Error other than "would block"
      _connected = false;
    }
  }

  return _connected ? 1 : 0;
}

// ============================================================================
// MacOSServer implementation
// ============================================================================

void MacOSServer::begin() {
  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket < 0) {
    Serial.println("Error creating socket");
    return;
  }

  // Allow address reuse for quick restarts
  int opt = 1;
  setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(_port);

  if (bind(_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    Serial.println("Error binding socket");
    close(_socket);
    _socket = -1;
    return;
  }

  if (listen(_socket, 5) < 0) {
    Serial.println("Error listening on socket");
    close(_socket);
    _socket = -1;
    return;
  }

  // Set non-blocking for accept
  int flags = fcntl(_socket, F_GETFL, 0);
  fcntl(_socket, F_SETFL, flags | O_NONBLOCK);
}

MacOSClient MacOSServer::available() {
  if (_socket < 0) {
    return MacOSClient();
  }

  struct sockaddr_in clientAddr;
  socklen_t clientLen = sizeof(clientAddr);

  int clientSocket = accept(_socket, (struct sockaddr*)&clientAddr, &clientLen);

  if (clientSocket < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // No connection waiting, return empty client
      return MacOSClient();
    }
    // Real error
    return MacOSClient();
  }

  // Set client socket to blocking mode
  int flags = fcntl(clientSocket, F_GETFL, 0);
  fcntl(clientSocket, F_SETFL, flags & ~O_NONBLOCK);

  return MacOSClient(clientSocket);
}
