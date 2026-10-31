#ifndef AWOT_CALLBACK_SLOT_H_
#define AWOT_CALLBACK_SLOT_H_

#include "aWOT_common.h"

namespace awot {
namespace detail {

#ifdef STD_FUNCTION_MIDDLEWARE
template <typename Callback>
class CallbackSlot {
 public:
  CallbackSlot() : m_revision(0), m_active(NULL), m_activeRevision(0) {}
  CallbackSlot(const CallbackSlot&) = delete;
  CallbackSlot& operator=(const CallbackSlot&) = delete;

  CallbackSlot& operator=(Callback callback) {
    ++m_revision;
    callback.swap(m_callback);
    return *this;
  }

  explicit operator bool() const {
    return m_callback || (m_active != NULL && m_activeRevision == m_revision);
  }

  template <typename... Args>
  void operator()(Args&... args) {
    if (m_active != NULL && m_activeRevision == m_revision) {
      (*m_active)(args...);
      return;
    }
    if (!m_callback) return;

    uint32_t revision = m_revision;
    Callback callback;
    callback.swap(m_callback);
    Callback* previousActive = m_active;
    uint32_t previousActiveRevision = m_activeRevision;
    m_active = &callback;
    m_activeRevision = revision;
    callback(args...);
    m_active = previousActive;
    m_activeRevision = previousActiveRevision;
    if (revision == m_revision) callback.swap(m_callback);
  }

 private:
  Callback m_callback;
  uint32_t m_revision;
  Callback* m_active;
  uint32_t m_activeRevision;
};
#endif

}
}

#endif
