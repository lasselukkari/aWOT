/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things
*/

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

  SseChannel() : m_onConnect(nullptr) { memset(m_active, 0, sizeof(m_active)); }

  void open(Response& res) {
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
        if (m_onConnect) m_onConnect(m_clients[i]);
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

  void onConnect(Callback callback) { m_onConnect = callback; }

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
  Callback m_onConnect;
};

}  // namespace awot

#endif
