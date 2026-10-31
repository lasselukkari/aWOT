/* Arduino timing, formatting and console support for native Windows. */

#include "Arduino.h"

#include <chrono>
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
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
  auto now = std::chrono::steady_clock::now();
  while (now < deadline) {
    // Windows sleeps can finish early because of the system timer resolution.
    // Round up the remaining interval so retries do not become a busy wait.
    auto remaining =
        std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
    if (remaining < deadline - now) remaining += std::chrono::milliseconds(1);
    std::this_thread::sleep_for(remaining);
    now = std::chrono::steady_clock::now();
  }
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
