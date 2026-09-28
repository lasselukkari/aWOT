/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things
*/

#ifndef AWOT_SSE_CHANNEL_H_
#define AWOT_SSE_CHANNEL_H_

#include "Response.h"

namespace awot {

template <typename TClient, int N>
class SseChannel {
 public:
#ifdef STD_FUNCTION_MIDDLEWARE
  typedef std::function<void(Print&)> Callback;
#else
  typedef void (*Callback)(Print&);
#endif

  SseChannel() : m_onConnect(nullptr) {
    memset(m_active, 0, sizeof(m_active));
  }

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
      if (!m_clients[i].connected()) { m_active[i] = false; continue; }
      sendData(m_clients[i], data);
    }
  }

  void send(const char* event, const char* data) {
    for (int i = 0; i < N; i++) {
      if (!m_active[i]) continue;
      if (!m_clients[i].connected()) { m_active[i] = false; continue; }
      m_clients[i].print("event: ");
      m_clients[i].print(event);
      m_clients[i].print("\n");
      sendData(m_clients[i], data);
    }
  }

  void forEach(Callback fn) {
    for (int i = 0; i < N; i++) {
      if (!m_active[i]) continue;
      if (!m_clients[i].connected()) { m_active[i] = false; continue; }
      fn(m_clients[i]);
    }
  }

  void onConnect(Callback callback) { m_onConnect = callback; }

 private:
  static void sendData(TClient& client, const char* data) {
    const char* line = data;
    while (true) {
      const char* end = line;
      while (*end && *end != '\r' && *end != '\n') ++end;
      client.print("data: ");
      client.write((const uint8_t*)line, (size_t)(end - line));
      client.print("\n");
      if (!*end) break;
      line = end + 1;
      if (*end == '\r' && *line == '\n') ++line;
    }
    client.print("\n");
  }

  TClient m_clients[N];
  bool m_active[N];
  Callback m_onConnect;
};

}  // namespace awot

#endif
