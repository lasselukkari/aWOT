#ifndef AWOT_CALLBACK_SLOT_H_
#define AWOT_CALLBACK_SLOT_H_

#include "aWOT_common.h"

#ifdef STD_FUNCTION_MIDDLEWARE
#include <atomic>
#include <memory>
#include <utility>
#endif

namespace awot {
namespace detail {

#ifdef STD_FUNCTION_MIDDLEWARE
template <typename Callback>
class CallbackSlot {
 public:
  CallbackSlot() : m_callback() {}
  CallbackSlot(const CallbackSlot&) = delete;
  CallbackSlot& operator=(const CallbackSlot&) = delete;

  CallbackSlot& operator=(Callback callback) {
    std::shared_ptr<Callback> replacement;
    if (callback) {
      replacement = std::make_shared<Callback>(std::move(callback));
    }
    std::atomic_store(&m_callback, std::move(replacement));
    return *this;
  }

  explicit operator bool() const {
    return static_cast<bool>(std::atomic_load(&m_callback));
  }

  template <typename... Args>
  void operator()(Args&... args) {
    // Keep this callback alive independently of concurrent or nested updates.
    // Atomic publication protects the slot; user code runs without its lock.
    std::shared_ptr<Callback> callback = std::atomic_load(&m_callback);
    if (callback) (*callback)(args...);
  }

 private:
#if defined(__cpp_lib_atomic_shared_ptr) && \
    __cpp_lib_atomic_shared_ptr >= 201711L
  std::atomic<std::shared_ptr<Callback>> m_callback;
#else
  std::shared_ptr<Callback> m_callback;
#endif
};
#endif

}
}

#endif
