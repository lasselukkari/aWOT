#ifndef AWOT_CHANNEL_ROOMS_H_
#define AWOT_CHANNEL_ROOMS_H_

#include <stdint.h>
#include <string.h>

namespace awot {
namespace detail {

// Fixed room registry and memberships shared by the channel implementations.
template <int N, int MAX_ROOMS>
class ChannelRooms {
  static_assert(N > 0, "N must be positive");
  static_assert(
      MAX_ROOMS > 0 && MAX_ROOMS <= 32,
      "MAX_ROOMS must be in 1..32 (room membership is a uint32_t bitmap)");

 public:
  ChannelRooms() : m_count(0) {
    for (int i = 0; i < N; ++i) m_memberships[i] = 0;
    for (int r = 0; r < MAX_ROOMS; ++r) m_names[r] = nullptr;
  }

  int defineRoom(const char* name) {
    if (!name) return -1;
    int roomIdx = findRoom(name);
    if (roomIdx >= 0) return roomIdx;
    if (m_count >= MAX_ROOMS) return -1;
    m_names[m_count] = name;
    return m_count++;
  }

  int findRoom(const char* name) const {
    if (!name) return -1;
    for (int r = 0; r < m_count; ++r) {
      if (strcmp(m_names[r], name) == 0) return r;
    }
    return -1;
  }

  const char* roomName(int roomIdx) const {
    return validRoom(roomIdx) ? m_names[roomIdx] : nullptr;
  }

  void join(int clientId, int roomIdx) {
    if (!validClient(clientId) || !validRoom(roomIdx)) return;
    m_memberships[clientId] |= (uint32_t(1) << roomIdx);
  }

  void join(int clientId, const char* name) { join(clientId, findRoom(name)); }

  void leave(int clientId, int roomIdx) {
    if (!validClient(clientId) || !validRoom(roomIdx)) return;
    m_memberships[clientId] &= ~(uint32_t(1) << roomIdx);
  }

  void leave(int clientId, const char* name) {
    leave(clientId, findRoom(name));
  }

  bool inRoom(int clientId, int roomIdx) const {
    if (!validClient(clientId) || !validRoom(roomIdx)) return false;
    return (m_memberships[clientId] & (uint32_t(1) << roomIdx)) != 0;
  }

  bool inRoom(int clientId, const char* name) const {
    return inRoom(clientId, findRoom(name));
  }

  void clearClient(int clientId) {
    if (validClient(clientId)) m_memberships[clientId] = 0;
  }

  bool validRoom(int roomIdx) const {
    return roomIdx >= 0 && roomIdx < MAX_ROOMS;
  }

 private:
  bool validClient(int clientId) const { return clientId >= 0 && clientId < N; }

  uint32_t m_memberships[N];
  const char* m_names[MAX_ROOMS];
  int m_count;
};

}  // namespace detail
}  // namespace awot

#endif
