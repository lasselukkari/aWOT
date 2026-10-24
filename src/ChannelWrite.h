#ifndef AWOT_CHANNEL_WRITE_H_
#define AWOT_CHANNEL_WRITE_H_

#include <stddef.h>
#include <stdint.h>

namespace awot {
namespace detail {

// Channels decide how a failed write affects their connection lifecycle.
template <typename TOutput>
bool writeAll(TOutput& output, const uint8_t* data, size_t length) {
  while (length) {
    size_t written = output.write(data, length);
    if (!written || written > length) return false;
    data += written;
    length -= written;
  }
  return true;
}

template <typename TOutput>
bool writeAll(TOutput& output, const char* data, size_t length) {
  return writeAll(output, reinterpret_cast<const uint8_t*>(data), length);
}

}  // namespace detail
}  // namespace awot

#endif
