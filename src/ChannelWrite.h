#ifndef AWOT_CHANNEL_WRITE_H_
#define AWOT_CHANNEL_WRITE_H_

#include <stddef.h>
#include <stdint.h>

namespace awot {
namespace detail {

struct WriteResult {
  size_t written;
  bool success;
};

template <typename TOutput>
WriteResult writeAllResult(TOutput& output, const uint8_t* data,
                           size_t length) {
  size_t written = 0;
  while (written < length) {
    size_t remaining = length - written;
    size_t count = output.write(data + written, remaining);
    if (!count || count > remaining) return {written, false};
    written += count;
  }
  return {written, true};
}

template <typename TOutput>
bool writeAll(TOutput& output, const uint8_t* data, size_t length) {
  return writeAllResult(output, data, length).success;
}

template <typename TOutput>
bool writeAll(TOutput& output, const char* data, size_t length) {
  return writeAll(output, reinterpret_cast<const uint8_t*>(data), length);
}

}
}

#endif
