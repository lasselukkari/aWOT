/*
  WsMessage: Stream-derived wrapper over a completed WebSocket frame payload.
  Handed to application message callbacks so they can consume payload through
  the same Stream interface aWOT uses elsewhere.
*/

#ifndef AWOT_WS_MESSAGE_H_
#define AWOT_WS_MESSAGE_H_

#include "aWOT_common.h"

namespace awot {

class WsMessage : public Stream {
 public:
  enum Opcode { TEXT = 0x1, BINARY = 0x2 };

  WsMessage(uint8_t* buffer, size_t length, uint8_t opcode, int socketId)
      : m_buffer(buffer),
        m_length(length),
        m_pos(0),
        m_opcode(opcode),
        m_socketId(socketId) {}

  int available() { return (int)(m_length - m_pos); }

  int read() {
    if (m_pos >= m_length) return -1;
    return m_buffer[m_pos++];
  }

  int peek() {
    if (m_pos >= m_length) return -1;
    return m_buffer[m_pos];
  }

  size_t write(uint8_t) { return 0; }

  void flush() {}

  int socketId() const { return m_socketId; }
  bool isText() const { return m_opcode == TEXT; }
  bool isBinary() const { return m_opcode == BINARY; }

  const uint8_t* data() const { return m_buffer; }
  size_t length() const { return m_length; }

 private:
  uint8_t* m_buffer;
  size_t m_length;
  size_t m_pos;
  uint8_t m_opcode;
  int m_socketId;
};

}  // namespace awot

#endif
