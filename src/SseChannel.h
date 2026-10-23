#ifndef AWOT_SSE_CHANNEL_H_
#define AWOT_SSE_CHANNEL_H_

#include "Response.h"

namespace awot {

template <typename TClient, int N>
class SseChannel {
  static_assert(N > 0, "N must be positive");

 public:
#ifdef STD_FUNCTION_MIDDLEWARE
  typedef std::function<void(Print&)> Callback;
#else
  typedef void (*Callback)(Print&);
#endif

  SseChannel()
#ifdef STD_FUNCTION_MIDDLEWARE
      : m_connectRevision(0), m_onConnect(nullptr) {
#else
      : m_onConnect(nullptr) {
#endif
    memset(m_active, 0, sizeof(m_active));
  }

  void open(Response& res) {
    // A stream without its required headers must not be retained by the caller.
    if (!res.m_canStartChannel(200, 2)) {
      res.m_failWrite();
      return;
    }
    res.status(200);
    res.set("Content-Type", "text/event-stream");
    res.set("Cache-Control", "no-cache");
    res.keepOpen();
  }

  void add(TClient client) {
    for (int i = 0; i < N; i++) {
      if (!m_active[i] || !m_clients[i].connected()) {
        if (m_active[i]) m_clients[i].stop();
        m_clients[i] = client;
        m_active[i] = true;
        callOnConnect(m_clients[i]);
        return;
      }
    }
    client.stop();
  }

  void send(const char* data) {
    for (int i = 0; i < N; i++) {
      if (!m_active[i]) continue;
      if (!m_clients[i].connected() || !sendData(m_clients[i], data)) {
        deactivate(i);
      }
    }
  }

  void send(const char* event, const char* data) {
    for (int i = 0; i < N; i++) {
      if (!m_active[i]) continue;
      if (!m_clients[i].connected() || !writeAll(m_clients[i], "event: ", 7) ||
          !writeAll(m_clients[i], event, strlen(event)) ||
          !writeAll(m_clients[i], "\n", 1) || !sendData(m_clients[i], data)) {
        deactivate(i);
      }
    }
  }

  void forEach(Callback fn) {
    for (int i = 0; i < N; i++) {
      if (!m_active[i]) continue;
      if (!m_clients[i].connected()) {
        deactivate(i);
        continue;
      }
      fn(m_clients[i]);
    }
  }

  void onConnect(Callback callback) {
#ifdef STD_FUNCTION_MIDDLEWARE
    ++m_connectRevision;
#endif
    m_onConnect = callback;
  }

 private:
  void deactivate(int i) {
    m_active[i] = false;
    m_clients[i].stop();
  }

  static bool writeAll(TClient& client, const char* data, size_t length) {
    while (length) {
      size_t written = client.write((const uint8_t*)data, length);
      if (!written || written > length) return false;
      data += written;
      length -= written;
    }
    return true;
  }

  static bool sendData(TClient& client, const char* data) {
    const char* line = data;
    while (true) {
      const char* end = line;
      while (*end && *end != '\r' && *end != '\n') ++end;
      if (!writeAll(client, "data: ", 6) ||
          !writeAll(client, line, (size_t)(end - line)) ||
          !writeAll(client, "\n", 1))
        return false;
      if (!*end) break;
      line = end + 1;
      if (*end == '\r' && *line == '\n') ++line;
    }
    return writeAll(client, "\n", 1);
  }

  TClient m_clients[N];
  bool m_active[N];
#ifdef STD_FUNCTION_MIDDLEWARE
  uint32_t m_connectRevision;
#endif
  Callback m_onConnect;

  void callOnConnect(Print& output) {
#ifdef STD_FUNCTION_MIDDLEWARE
    uint32_t revision = m_connectRevision;
    Callback callback;
    callback.swap(m_onConnect);
    if (!callback) return;
    callback(output);
    // Restore the invoked callback unless it replaced itself.
    if (revision == m_connectRevision) callback.swap(m_onConnect);
#else
    if (m_onConnect) m_onConnect(output);
#endif
  }
};

}  // namespace awot

#endif
