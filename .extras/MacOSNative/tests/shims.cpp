#include <atomic>
#include <chrono>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <thread>
#include <vector>

#include "Arduino.h"

namespace {

void check(bool condition, const char* message) {
  if (!condition) {
    std::fprintf(stderr, "Shim test failed: %s\n", message);
    std::exit(EXIT_FAILURE);
  }
}

void testTimers() {
  // Make the first timer calls concurrently, including both units.
  std::atomic<int> ready(0);
  std::atomic<bool> start(false);
  std::atomic<bool> valid(true);
  std::vector<std::thread> threads;
  for (int i = 0; i < 8; ++i) {
    threads.emplace_back([&]() {
      ++ready;
      while (!start.load()) std::this_thread::yield();
      unsigned long previousMillis = millis();
      unsigned long previousMicros = micros();
      for (int sample = 0; sample < 1000; ++sample) {
        const unsigned long before = millis();
        const unsigned long microseconds = micros();
        const unsigned long after = millis();
        if (before < previousMillis || microseconds < previousMicros ||
            microseconds / 1000 < before || microseconds / 1000 > after) {
          valid = false;
        }
        previousMillis = after;
        previousMicros = microseconds;
      }
    });
  }
  while (ready.load() < 8) std::this_thread::yield();
  start = true;
  for (auto& thread : threads) thread.join();
  check(valid, "millis and micros must be monotonic and share an origin");

  // Repeat intervals around Windows timer ticks to catch early wakeups.
  const unsigned long delays[] = {0, 1, 2, 15, 16, 31};
  for (int sample = 0; sample < 8; ++sample) {
    for (unsigned long ms : delays) {
      const auto before = std::chrono::steady_clock::now();
      delay(ms);
      const auto elapsed = std::chrono::steady_clock::now() - before;
      if (elapsed < std::chrono::milliseconds(ms)) {
        std::fprintf(
            stderr, "delay(%lu) returned after %lld microseconds\n", ms,
            static_cast<long long>(
                std::chrono::duration_cast<std::chrono::microseconds>(elapsed)
                    .count()));
      }
      check(elapsed >= std::chrono::milliseconds(ms),
            "delay must wait for the requested duration");
    }
  }
  yield();
}

void testIntegers() {
  char buffer[CHAR_BIT * sizeof(unsigned long) + 2];
  check(std::strcmp(itoa(42, buffer, 2), "101010") == 0,
        "itoa must support binary");
  check(std::strcmp(ltoa(42L, buffer, 8), "52") == 0,
        "ltoa must support octal");
  check(std::strcmp(utoa(42U, buffer, 8), "52") == 0,
        "utoa must support octal");
  check(std::strcmp(ultoa(42UL, buffer, 8), "52") == 0,
        "ultoa must support octal");
  check(std::strcmp(ultoa(71, buffer, 36), "1z") == 0,
        "base 36 must include alphabetic digits");

  for (int base = 2; base <= 36; ++base) {
    check(itoa(INT_MAX, buffer, base) == buffer,
          "itoa must return its output buffer");
    check(std::strtoul(buffer, nullptr, base) == INT_MAX,
          "itoa must preserve the value in every supported radix");
    ltoa(LONG_MAX, buffer, base);
    check(std::strtoul(buffer, nullptr, base) == LONG_MAX,
          "ltoa must preserve the value in every supported radix");
    utoa(UINT_MAX, buffer, base);
    check(std::strtoul(buffer, nullptr, base) == UINT_MAX,
          "utoa must preserve the value in every supported radix");
    ultoa(ULONG_MAX, buffer, base);
    check(std::strtoul(buffer, nullptr, base) == ULONG_MAX,
          "ultoa must preserve the value in every supported radix");
    check(std::strcmp(ultoa(0, buffer, base), "0") == 0,
          "zero must have one digit in every supported radix");
  }

  check(std::string(itoa(INT_MIN, buffer, 10)) == std::to_string(INT_MIN),
        "itoa must handle the signed minimum without overflow");
  check(std::string(ltoa(LONG_MIN, buffer, 10)) == std::to_string(LONG_MIN),
        "ltoa must handle the signed minimum without overflow");
  check(std::string(itoa(-1, buffer, 2)) ==
            std::string(CHAR_BIT * sizeof(unsigned), '1'),
        "negative nondecimal ints must use their unsigned representation");
  check(std::string(ltoa(-1, buffer, 2)) ==
            std::string(CHAR_BIT * sizeof(unsigned long), '1'),
        "negative nondecimal longs must not truncate their binary digits");
  for (int base : {-1, 0, 1, 37}) {
    check(itoa(-42, buffer, base)[0] == '\0', "reject invalid itoa radices");
    check(ltoa(-42, buffer, base)[0] == '\0', "reject invalid ltoa radices");
    check(utoa(42, buffer, base)[0] == '\0', "reject invalid utoa radices");
    check(ultoa(42, buffer, base)[0] == '\0', "reject invalid ultoa radices");
  }

  check(arduino::String(42, 2) == "101010", "String must support binary ints");
  check(arduino::String(42UL, 8) == "52", "String must support octal longs");
}

void testFloatingPoint() {
  char buffer[1024];
  char expected[1024];
  check(dtostrf(1.25, 8, 2, buffer) == buffer,
        "dtostrf must return its output buffer");
  check(std::strcmp(buffer, "    1.25") == 0,
        "positive width must right align");
  check(std::strcmp(dtostrf(1.25, -8, 2, buffer), "1.25    ") == 0,
        "negative width must left align");
  check(std::strcmp(dtostrf(1.75, 0, 0, buffer), "2") == 0,
        "zero precision must round to an integer");
  check(std::strcmp(dtostrf(-1.25, 0, 2, buffer), "-1.25") == 0,
        "dtostrf must preserve a negative sign");

  const double values[] = {1e40, std::numeric_limits<double>::max(),
                           -std::numeric_limits<double>::max()};
  for (double value : values) {
    std::snprintf(expected, sizeof(expected), "%.2f", value);
    check(std::strcmp(dtostrf(value, 0, 2, buffer), expected) == 0,
          "dtostrf must not truncate large values");
    check(arduino::String(value, 2) == expected,
          "String must preserve large floating-point values");
    arduino::String concatenated("value=");
    check(concatenated.concat(value), "double concatenation must succeed");
    check(concatenated == (std::string("value=") + expected).c_str(),
          "String concatenation must preserve large doubles");
    std::snprintf(expected, sizeof(expected), "%.10f", value);
    check(arduino::String(value, 10) == expected,
          "String must fit double limits with maximum precision");
  }

  const float floatValues[] = {std::numeric_limits<float>::max(),
                               -std::numeric_limits<float>::max()};
  for (float value : floatValues) {
    std::snprintf(expected, sizeof(expected), "%.10f", value);
    check(arduino::String(value, 10) == expected,
          "String must fit float limits with maximum precision");
    std::snprintf(expected, sizeof(expected), "%.2f", value);
    arduino::String concatenated("value=");
    check(concatenated.concat(value), "float concatenation must succeed");
    check(concatenated == (std::string("value=") + expected).c_str(),
          "String concatenation must preserve large floats");
  }

  std::snprintf(expected, sizeof(expected), "%80.50f", 1.25);
  check(std::strcmp(dtostrf(1.25, 80, 50, buffer), expected) == 0,
        "dtostrf must not truncate wide or high-precision output");
}

}  // namespace

int main() {
  testTimers();
  testIntegers();
  testFloatingPoint();
  std::puts("Shim tests passed");
  return EXIT_SUCCESS;
}
