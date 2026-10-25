#ifndef AWOT_ARRAY_VIEW_H_
#define AWOT_ARRAY_VIEW_H_

#include <limits.h>
#include <stddef.h>

namespace awot {

template <typename T>
class ArrayView {
 public:
  ArrayView() : m_data(nullptr), m_size(0) {}
  ArrayView(T* data, int size) : m_data(data), m_size(size) {}

  template <size_t N>
  ArrayView(T (&data)[N]) : m_data(data), m_size(static_cast<int>(N)) {
    static_assert(N <= INT_MAX, "Array capacity exceeds INT_MAX");
  }

  T* data() const { return m_data; }
  int size() const { return m_size; }
  T& operator[](int index) const { return m_data[index]; }

 private:
  T* m_data;
  int m_size;
};

}

#endif
