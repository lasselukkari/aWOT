#ifndef AWOT_SSE_CHANNEL_H_
#define AWOT_SSE_CHANNEL_H_

#include "ChannelRooms.h"
#include "ChannelWrite.h"
#include "Response.h"

namespace awot {

template <typename TClient, int N, int MAX_ROOMS = 8>
class SseChannel {
  static_assert(N > 0, "N must be positive");

 public:
#ifdef STD_FUNCTION_MIDDLEWARE
  typedef std::function<void(Print&)> Callback;
  typedef std::function<void(int clientId)> ConnectCb;
#else
  typedef void (*Callback)(Print&);
  typedef void (*ConnectCb)(int clientId);
#endif

  SseChannel()
#ifdef STD_FUNCTION_MIDDLEWARE
      : m_connectRevision(0),
        m_activeConnect(nullptr),
        m_activeConnectRevision(0),
        m_onConnect(nullptr) {
#else
      : m_onConnect(nullptr) {
#endif
    memset(m_active, 0, sizeof(m_active));
  }

  void open(Response& res) {
    if (!res.m_canStartChannel(200, 2)) {
      res.m_failWrite();
      return;
    }
    res.status(200);
    res.set("Content-Type", "text/event-stream");
    res.set("Cache-Control", "no-cache");
    res.keepOpen();
  }

  int add(TClient client) {
    for (int i = 0; i < N; i++) {
      if (!m_active[i] || !m_clients[i].connected()) {
        if (m_active[i]) deactivate(i);
        m_rooms.clearClient(i);
        m_clients[i] = client;
        m_active[i] = true;
        callOnConnect(i);
        return i;
      }
    }
    client.stop();
    return -1;
  }

  int defineRoom(const char* name) { return m_rooms.defineRoom(name); }

  int findRoom(const char* name) const { return m_rooms.findRoom(name); }

  const char* roomName(int roomIdx) const { return m_rooms.roomName(roomIdx); }

  void join(int clientId, int roomIdx) { m_rooms.join(clientId, roomIdx); }

  void join(int clientId, const char* name) { m_rooms.join(clientId, name); }

  void leave(int clientId, int roomIdx) { m_rooms.leave(clientId, roomIdx); }

  void leave(int clientId, const char* name) { m_rooms.leave(clientId, name); }

  bool inRoom(int clientId, int roomIdx) const {
    return m_rooms.inRoom(clientId, roomIdx);
  }

  bool inRoom(int clientId, const char* name) const {
    return m_rooms.inRoom(clientId, name);
  }

  void send(int clientId, const char* data) {
    sendEvent(clientId, nullptr, data);
  }

  void send(int clientId, const char* event, const char* data) {
    sendEvent(clientId, event, data);
  }

  void to(int roomIdx, const char* data) { to(roomIdx, nullptr, data); }

  void to(const char* name, const char* data) { to(findRoom(name), data); }

  void to(int roomIdx, const char* event, const char* data) {
    if (!m_rooms.validRoom(roomIdx)) return;
    for (int i = 0; i < N; i++) {
      if (m_rooms.inRoom(i, roomIdx)) sendEvent(i, event, data);
    }
  }

  void to(const char* name, const char* event, const char* data) {
    to(findRoom(name), event, data);
  }

  void broadcast(const char* data) { broadcast(nullptr, data); }

  void broadcast(const char* event, const char* data) {
    for (int i = 0; i < N; i++) {
      sendEvent(i, event, data);
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

  int count() const {
    int c = 0;
    for (int i = 0; i < N; i++)
      if (m_active[i]) c++;
    return c;
  }

  bool connected(int clientId) const {
    return validClient(clientId) && m_active[clientId];
  }

  void close(int clientId) {
    if (!connected(clientId)) return;
    deactivate(clientId);
  }

  void onConnect(ConnectCb callback) {
#ifdef STD_FUNCTION_MIDDLEWARE
    ++m_connectRevision;
#endif
    m_onConnect = callback;
  }

 private:
  bool validClient(int clientId) const { return clientId >= 0 && clientId < N; }

  void deactivate(int i) {
    m_active[i] = false;
    m_rooms.clearClient(i);
    m_clients[i].stop();
  }

  void sendEvent(int clientId, const char* event, const char* data) {
    if (!connected(clientId)) return;
    TClient& client = m_clients[clientId];
    if (!client.connected() ||
        (event && (!detail::writeAll(client, "event: ", 7) ||
                   !detail::writeAll(client, event, strlen(event)) ||
                   !detail::writeAll(client, "\n", 1))) ||
        !sendData(client, data)) {
      deactivate(clientId);
    }
  }

  static bool sendData(TClient& client, const char* data) {
    const char* line = data;
    while (true) {
      const char* end = line;
      while (*end && *end != '\r' && *end != '\n') ++end;
      if (!detail::writeAll(client, "data: ", 6) ||
          !detail::writeAll(client, line, (size_t)(end - line)) ||
          !detail::writeAll(client, "\n", 1))
        return false;
      if (!*end) break;
      line = end + 1;
      if (*end == '\r' && *line == '\n') ++line;
    }
    return detail::writeAll(client, "\n", 1);
  }

  TClient m_clients[N];
  bool m_active[N];
  detail::ChannelRooms<N, MAX_ROOMS> m_rooms;
#ifdef STD_FUNCTION_MIDDLEWARE
  uint32_t m_connectRevision;
  ConnectCb* m_activeConnect;
  uint32_t m_activeConnectRevision;
#endif
  ConnectCb m_onConnect;

  void callOnConnect(int clientId) {
#ifdef STD_FUNCTION_MIDDLEWARE
    if (m_activeConnect && m_activeConnectRevision == m_connectRevision) {
      (*m_activeConnect)(clientId);
      return;
    }
    if (!m_onConnect) return;
    uint32_t revision = m_connectRevision;
    ConnectCb callback;
    callback.swap(m_onConnect);
    ConnectCb* previousActive = m_activeConnect;
    uint32_t previousActiveRevision = m_activeConnectRevision;
    m_activeConnect = &callback;
    m_activeConnectRevision = revision;
    callback(clientId);
    m_activeConnect = previousActive;
    m_activeConnectRevision = previousActiveRevision;
    if (revision == m_connectRevision) callback.swap(m_onConnect);
#else
    if (m_onConnect) m_onConnect(clientId);
#endif
  }
};

}

#endif
