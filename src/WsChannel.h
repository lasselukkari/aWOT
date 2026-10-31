#ifndef AWOT_WS_CHANNEL_H_
#define AWOT_WS_CHANNEL_H_

#include "App.h"
#include "CallbackSlot.h"
#include "ChannelRooms.h"
#include "ChannelWrite.h"
#include "Request.h"
#include "Response.h"
#include "WsCrypto.h"
#include "WsMessage.h"

#ifndef WS_RX_BUFFER_SIZE
#define WS_RX_BUFFER_SIZE 256
#endif

#ifndef WS_PING_INTERVAL_MS
#define WS_PING_INTERVAL_MS 5000UL
#endif

#ifndef WS_PING_TIMEOUT_MS
#define WS_PING_TIMEOUT_MS 10000UL
#endif

namespace awot {

template <typename TClient, int N, int MAX_ROOMS = 8>
class WsChannel {
  static_assert(N > 0, "N must be positive");
  static_assert(
      MAX_ROOMS > 0 && MAX_ROOMS <= 32,
      "MAX_ROOMS must be in 1..32 (room membership is a uint32_t bitmap)");

 public:
#ifdef STD_FUNCTION_MIDDLEWARE
  typedef std::function<void(int socketId)> ConnectCb;
  typedef std::function<void(int socketId)> DisconnectCb;
  typedef std::function<void(WsMessage& msg)> MessageCb;
#else
  typedef void (*ConnectCb)(int socketId);
  typedef void (*DisconnectCb)(int socketId);
  typedef void (*MessageCb)(WsMessage& msg);
#endif

  WsChannel()
      : m_polling(false), m_onConnect(), m_onDisconnect(), m_onMessage() {
    for (int i = 0; i < N; i++) {
      m_slots[i].active = false;
      resetRxState(i);
    }
  }

  bool upgrade(Request& req, Response& res) {
    const char* upgrade = req.get("Upgrade");
    const char* connection = req.get("Connection");
    const char* key = req.get("Sec-WebSocket-Key");
    const char* version = req.get("Sec-WebSocket-Version");

    if (req.method() != Request::GET || req.minorVersion() != 1 || req.left()) {
      return false;
    }
    if (!upgrade || !connection || !key || !version) return false;
    if (!matchesValue(upgrade, "websocket")) return false;
    if (!containsToken(connection, "upgrade")) return false;
    if (!matchesValue(version, "13") || !validKey(key)) return false;

    Client* c = req.client();
    if (!c) return false;
    if (!res.m_canStartChannel(101, 2)) return false;

    int slot = findFreeSlot();
    if (slot < 0) {
      return false;
    }

    char accept[32];
    ws::computeAcceptKey(key, accept);

    res.status(101);
    res.set("Upgrade", "websocket");
    res.set("Connection", "Upgrade");
    res.beginHeaders();
    P(acceptHeader) = "Sec-WebSocket-Accept: ";
    res.printP(acceptHeader);
    res.print(accept);
    res.m_printCRLF();
    res.endHeaders();
    if (res.getWriteError()) return false;
    res.keepOpen();

    m_slots[slot].client = *static_cast<TClient*>(c);
    m_slots[slot].active = true;
    m_rooms.clearClient(slot);
    resetRxState(slot);
    uint32_t now = millis();
    m_slots[slot].lastPingSentMs = now;
    m_slots[slot].awaitingPong = false;

    if (m_onConnect) m_onConnect(slot);
    return true;
  }

  void poll() {
    if (m_polling) return;
    struct PollGuard {
      bool& polling;
      ~PollGuard() { polling = false; }
    } guard = {m_polling};
    m_polling = true;
    for (int i = 0; i < N; i++) {
      if (m_slots[i].active) pollSlot(i);
    }
  }

  int defineRoom(const char* name) { return m_rooms.defineRoom(name); }

  int findRoom(const char* name) const { return m_rooms.findRoom(name); }

  const char* roomName(int roomIdx) const { return m_rooms.roomName(roomIdx); }

  void join(int socketId, int roomIdx) { m_rooms.join(socketId, roomIdx); }

  void join(int socketId, const char* name) { join(socketId, findRoom(name)); }

  void leave(int socketId, int roomIdx) { m_rooms.leave(socketId, roomIdx); }

  void leave(int socketId, const char* name) {
    leave(socketId, findRoom(name));
  }

  bool inRoom(int socketId, int roomIdx) const {
    return m_rooms.inRoom(socketId, roomIdx);
  }

  bool inRoom(int socketId, const char* name) const {
    return inRoom(socketId, findRoom(name));
  }

  void send(int socketId, const char* text) {
    if (!validSocket(socketId) || !m_slots[socketId].active) return;
    sendFrame(socketId, 0x1, (const uint8_t*)text, strlen(text));
  }

  void send(int socketId, const uint8_t* data, size_t n, bool binary = false) {
    if (!validSocket(socketId) || !m_slots[socketId].active) return;
    sendFrame(socketId, binary ? 0x2 : 0x1, data, n);
  }

  void to(int roomIdx, const char* text) {
    if (!validRoom(roomIdx)) return;
    size_t len = strlen(text);
    for (int i = 0; i < N; i++) {
      if (m_slots[i].active && m_rooms.inRoom(i, roomIdx)) {
        sendFrame(i, 0x1, (const uint8_t*)text, len);
      }
    }
  }

  void to(const char* name, const char* text) { to(findRoom(name), text); }

  void to(int roomIdx, const uint8_t* data, size_t n, bool binary = false) {
    if (!validRoom(roomIdx)) return;
    uint8_t op = binary ? 0x2 : 0x1;
    for (int i = 0; i < N; i++) {
      if (m_slots[i].active && m_rooms.inRoom(i, roomIdx)) {
        sendFrame(i, op, data, n);
      }
    }
  }

  void to(const char* name, const uint8_t* data, size_t n,
          bool binary = false) {
    to(findRoom(name), data, n, binary);
  }

  void broadcast(const char* text) {
    size_t len = strlen(text);
    for (int i = 0; i < N; i++) {
      if (m_slots[i].active) sendFrame(i, 0x1, (const uint8_t*)text, len);
    }
  }

  void broadcast(const uint8_t* data, size_t n, bool binary = false) {
    uint8_t op = binary ? 0x2 : 0x1;
    for (int i = 0; i < N; i++) {
      if (m_slots[i].active) sendFrame(i, op, data, n);
    }
  }

  int count() const {
    int c = 0;
    for (int i = 0; i < N; i++)
      if (m_slots[i].active) c++;
    return c;
  }

  bool connected(int socketId) const {
    return validSocket(socketId) && m_slots[socketId].active;
  }

  void close(int socketId, uint16_t code = 1000) {
    if (!validSocket(socketId) || !m_slots[socketId].active) return;
    closeSlot(socketId, validCloseCode(code) ? code : 1002);
  }

  void onConnect(ConnectCb cb) { m_onConnect = cb; }
  void onDisconnect(DisconnectCb cb) { m_onDisconnect = cb; }
  void onMessage(MessageCb cb) { m_onMessage = cb; }

 private:
  struct Slot {
    TClient client;
    bool active;
    uint8_t rxHeader[14];
    uint8_t rxHeaderFilled;
    uint8_t rxHeaderNeeded;
    bool rxHeaderDecoded;
    bool rxPayloadResolved;
    uint8_t rxOpcode;
    uint8_t rxMaskKey[4];
    uint32_t rxExpected;
    uint32_t rxFilledPayload;
    size_t rxMessageLength;
    uint8_t rxMessageOpcode;
    uint8_t rxPayload[WS_RX_BUFFER_SIZE];
    uint8_t rxControl[125];
    uint32_t lastPingSentMs;
    bool awaitingPong;
  };

  Slot m_slots[N];
  detail::ChannelRooms<N, MAX_ROOMS> m_rooms;
  bool m_polling;

#ifdef STD_FUNCTION_MIDDLEWARE
  detail::CallbackSlot<ConnectCb> m_onConnect;
  detail::CallbackSlot<DisconnectCb> m_onDisconnect;
  detail::CallbackSlot<MessageCb> m_onMessage;
#else
  ConnectCb m_onConnect;
  DisconnectCb m_onDisconnect;
  MessageCb m_onMessage;
#endif

  bool validRoom(int r) const { return m_rooms.validRoom(r); }
  bool validSocket(int s) const { return s >= 0 && s < N; }

  int findFreeSlot() {
    for (int i = 0; i < N; i++) {
      if (!m_slots[i].active) return i;
      if (!m_slots[i].client.connected()) {
        deactivate(i);
        if (!m_slots[i].active) return i;
      }
    }
    for (int i = 0; i < N; i++) {
      if (!m_slots[i].active) return i;
    }
    return -1;
  }

  void resetRxState(int i) {
    resetRxFrame(i);
    m_slots[i].rxMessageLength = 0;
    m_slots[i].rxMessageOpcode = 0;
  }

  void resetRxFrame(int i) {
    m_slots[i].rxHeaderFilled = 0;
    m_slots[i].rxHeaderNeeded = 2;
    m_slots[i].rxHeaderDecoded = false;
    m_slots[i].rxPayloadResolved = false;
    m_slots[i].rxExpected = 0;
    m_slots[i].rxFilledPayload = 0;
    m_slots[i].rxOpcode = 0;
  }

  void deactivate(int i) {
    if (!m_slots[i].active) return;
    m_slots[i].active = false;
    resetRxState(i);
    m_rooms.clearClient(i);
    m_slots[i].client.stop();
    if (m_onDisconnect) m_onDisconnect(i);
  }

  void closeSlot(int i, uint16_t code) {
    if (!m_slots[i].active) return;
    uint8_t payload[2];
    payload[0] = (uint8_t)(code >> 8);
    payload[1] = (uint8_t)(code & 0xFF);
    writeFrame(m_slots[i].client, 0x8, payload, 2);
    deactivate(i);
  }

  bool writeFrame(TClient& c, uint8_t opcode, const uint8_t* data, size_t len) {
    if (!c.connected()) return false;
    uint8_t h[10];
    int hl = 2;
    h[0] = 0x80 | (opcode & 0x0F);
    if (len <= 125) {
      h[1] = (uint8_t)len;
    } else if (len <= 0xFFFF) {
      h[1] = 126;
      h[2] = (uint8_t)(len >> 8);
      h[3] = (uint8_t)(len & 0xFF);
      hl = 4;
    } else {
      h[1] = 127;
      h[2] = h[3] = h[4] = h[5] = 0;
      h[6] = (uint8_t)(((uint32_t)len >> 24) & 0xFF);
      h[7] = (uint8_t)(((uint32_t)len >> 16) & 0xFF);
      h[8] = (uint8_t)((len >> 8) & 0xFF);
      h[9] = (uint8_t)(len & 0xFF);
      hl = 10;
    }
    return detail::writeAll(c, h, hl) && detail::writeAll(c, data, len);
  }

  void sendFrame(int i, uint8_t opcode, const uint8_t* data, size_t len) {
    if (!writeFrame(m_slots[i].client, opcode, data, len)) {
      deactivate(i);
    }
  }

  void pollSlot(int i) {
    Slot& s = m_slots[i];
    if (!s.client.connected()) {
      deactivate(i);
      return;
    }

    bool progress = true;
    while (progress) {
      progress = false;

      while (s.rxHeaderFilled < 2 && s.client.available() > 0) {
        int b = s.client.read();
        if (b < 0) break;
        s.rxHeader[s.rxHeaderFilled++] = (uint8_t)b;
        progress = true;
      }
      if (s.rxHeaderFilled < 2) break;

      if (!s.rxHeaderDecoded) {
        uint8_t fin = s.rxHeader[0] & 0x80;
        uint8_t rsv = s.rxHeader[0] & 0x70;
        s.rxOpcode = s.rxHeader[0] & 0x0F;
        uint8_t masked = s.rxHeader[1] & 0x80;
        uint8_t len7 = s.rxHeader[1] & 0x7F;

        if ((s.rxOpcode & 0x8) && (!fin || len7 > 125)) {
          closeSlot(i, 1002);
          return;
        }
        if (rsv) {
          closeSlot(i, 1002);
          return;
        }
        if (!masked) {
          closeSlot(i, 1002);
          return;
        }
        if (s.rxOpcode == 0x0) {
          if (!s.rxMessageOpcode) {
            closeSlot(i, 1002);
            return;
          }
        } else if (s.rxOpcode == 0x1 || s.rxOpcode == 0x2) {
          if (s.rxMessageOpcode) {
            closeSlot(i, 1002);
            return;
          }
          s.rxMessageOpcode = s.rxOpcode;
        } else if (s.rxOpcode != 0x8 && s.rxOpcode != 0x9 &&
                   s.rxOpcode != 0xA) {
          closeSlot(i, 1002);
          return;
        }

        int need = 2 + 4;
        if (len7 == 126)
          need += 2;
        else if (len7 == 127)
          need += 8;
        s.rxHeaderNeeded = (uint8_t)need;
        s.rxHeaderDecoded = true;
      }

      while (s.rxHeaderFilled < s.rxHeaderNeeded && s.client.available() > 0) {
        int b = s.client.read();
        if (b < 0) break;
        s.rxHeader[s.rxHeaderFilled++] = (uint8_t)b;
        progress = true;
      }
      if (s.rxHeaderFilled < s.rxHeaderNeeded) break;

      if (!s.rxPayloadResolved) {
        uint8_t len7 = s.rxHeader[1] & 0x7F;
        uint32_t len = 0;
        int off = 2;
        if (len7 <= 125) {
          len = len7;
        } else if (len7 == 126) {
          len = ((uint32_t)s.rxHeader[2] << 8) | s.rxHeader[3];
          off = 4;
          if (len < 126) {
            closeSlot(i, 1002);
            return;
          }
        } else {
          if (s.rxHeader[2] & 0x80) {
            closeSlot(i, 1002);
            return;
          }
          for (int k = 0; k < 4; k++) {
            if (s.rxHeader[2 + k] != 0) {
              closeSlot(i, 1009);
              return;
            }
          }
          len = ((uint32_t)s.rxHeader[6] << 24) |
                ((uint32_t)s.rxHeader[7] << 16) |
                ((uint32_t)s.rxHeader[8] << 8) | s.rxHeader[9];
          off = 10;
          if (len <= 0xFFFF) {
            closeSlot(i, 1002);
            return;
          }
        }
        if (!(s.rxOpcode & 0x8) &&
            len > WS_RX_BUFFER_SIZE - s.rxMessageLength) {
          closeSlot(i, 1009);
          return;
        }
        memcpy(s.rxMaskKey, &s.rxHeader[off], 4);
        s.rxExpected = len;
        s.rxPayloadResolved = true;
      }

      uint8_t* payload =
          (s.rxOpcode & 0x8) ? s.rxControl : s.rxPayload + s.rxMessageLength;
      while (s.rxFilledPayload < s.rxExpected && s.client.available() > 0) {
        uint32_t remaining = s.rxExpected - s.rxFilledPayload;
        int avail = s.client.available();
        uint32_t chunk =
            (uint32_t)avail < remaining ? (uint32_t)avail : remaining;
        int got = s.client.read(payload + s.rxFilledPayload, chunk);
        if (got <= 0) break;
        s.rxFilledPayload += (uint32_t)got;
        progress = true;
      }
      if (s.rxFilledPayload < s.rxExpected) break;

      for (uint32_t k = 0; k < s.rxExpected; k++) {
        payload[k] ^= s.rxMaskKey[k & 3];
      }
      dispatchFrame(i, payload);
      if (!s.active) return;

      resetRxFrame(i);
      progress = true;
    }

    uint32_t now = millis();
    if (s.awaitingPong && (now - s.lastPingSentMs) >= WS_PING_TIMEOUT_MS) {
      deactivate(i);
      return;
    }
    if (!s.awaitingPong && (now - s.lastPingSentMs) >= WS_PING_INTERVAL_MS) {
      bool ok = writeFrame(s.client, 0x9, nullptr, 0);
      if (!ok) {
        deactivate(i);
        return;
      }
      s.lastPingSentMs = now;
      s.awaitingPong = true;
    }
  }

  void dispatchFrame(int i, uint8_t* payload) {
    Slot& s = m_slots[i];
    switch (s.rxOpcode) {
      case 0x0:
      case 0x1:
      case 0x2: {
        s.rxMessageLength += (size_t)s.rxExpected;
        if (!(s.rxHeader[0] & 0x80)) break;

        size_t length = s.rxMessageLength;
        uint8_t opcode = s.rxMessageOpcode;
        s.rxMessageLength = 0;
        s.rxMessageOpcode = 0;
        if (opcode == 0x1 && !validUtf8(s.rxPayload, length)) {
          closeSlot(i, 1007);
          return;
        }
        // Retire the frame before user code can throw or reuse this slot.
        resetRxFrame(i);
        if (m_onMessage) {
          WsMessage msg(s.rxPayload, length, opcode, i);
          m_onMessage(msg);
        }
        break;
      }
      case 0x8: {
        if (s.rxExpected == 1) {
          closeSlot(i, 1002);
          return;
        }
        if (s.rxExpected >= 2) {
          uint16_t code = ((uint16_t)payload[0] << 8) | payload[1];
          if (!validCloseCode(code)) {
            closeSlot(i, 1002);
            return;
          }
          if (!validUtf8(payload + 2, s.rxExpected - 2)) {
            closeSlot(i, 1007);
            return;
          }
        }
        writeFrame(s.client, 0x8, payload, s.rxExpected);
        deactivate(i);
        break;
      }
      case 0x9:
        sendFrame(i, 0xA, payload, s.rxExpected);
        break;
      case 0xA:
        if (s.awaitingPong) {
          s.awaitingPong = false;
          s.lastPingSentMs = millis();
        }
        break;
      default:
        closeSlot(i, 1003);
        break;
    }
  }

  static bool validCloseCode(uint16_t code) {
    return (code >= 1000 && code <= 1014 && code != 1004 && code != 1005 &&
            code != 1006) ||
           (code >= 3000 && code <= 4999);
  }

  static bool validUtf8(const uint8_t* data, size_t length) {
    size_t i = 0;
    while (i < length) {
      uint8_t first = data[i++];
      if (first < 0x80) continue;
      uint8_t remaining;
      uint8_t low = 0x80, high = 0xBF;
      if (first >= 0xC2 && first <= 0xDF) {
        remaining = 1;
      } else if (first >= 0xE0 && first <= 0xEF) {
        remaining = 2;
        if (first == 0xE0) low = 0xA0;
        if (first == 0xED) high = 0x9F;
      } else if (first >= 0xF0 && first <= 0xF4) {
        remaining = 3;
        if (first == 0xF0) low = 0x90;
        if (first == 0xF4) high = 0x8F;
      } else {
        return false;
      }
      if (length - i < remaining || data[i] < low || data[i] > high)
        return false;
      ++i;
      while (--remaining) {
        if (data[i] < 0x80 || data[i] > 0xBF) return false;
        ++i;
      }
    }
    return true;
  }

  static bool validKey(const char* key) {
    while (*key == ' ' || *key == '\t') ++key;
    for (int i = 0; i < 22; ++i) {
      char c = key[i];
      if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '+' || c == '/'))
        return false;
    }
    if (key[21] != 'A' && key[21] != 'Q' && key[21] != 'g' && key[21] != 'w') {
      return false;
    }
    if (key[22] != '=' || key[23] != '=') return false;
    key += 24;
    while (*key == ' ' || *key == '\t') ++key;
    return !*key;
  }

  static bool matchesValue(const char* value, const char* expected) {
    while (*value == ' ' || *value == '\t') ++value;
    while (*expected) {
      if (tolower((unsigned char)*value++) !=
          tolower((unsigned char)*expected++)) {
        return false;
      }
    }
    while (*value == ' ' || *value == '\t') ++value;
    return !*value;
  }

  static bool containsToken(const char* haystack, const char* needle) {
    if (!haystack || !needle || !*needle) return false;
    size_t length = strlen(needle);
    const char* start = haystack;
    while (*start) {
      while (*start == ' ' || *start == '\t') ++start;
      const char* end = start;
      while (*end && *end != ',') ++end;
      const char* trimmedEnd = end;
      while (trimmedEnd > start &&
             (trimmedEnd[-1] == ' ' || trimmedEnd[-1] == '\t'))
        --trimmedEnd;
      if ((size_t)(trimmedEnd - start) == length) {
        size_t i = 0;
        while (i < length && tolower((unsigned char)start[i]) ==
                                 tolower((unsigned char)needle[i]))
          ++i;
        if (i == length) return true;
      }
      start = *end ? end + 1 : end;
    }
    return false;
  }
};

}

#endif
