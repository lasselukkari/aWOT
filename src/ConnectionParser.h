#ifndef AWOT_CONNECTION_PARSER_H_
#define AWOT_CONNECTION_PARSER_H_

#include "App.h"

namespace awot {
namespace detail {

class ConnectionParser {
 public:
  ConnectionParser()
      : close(false), keepAlive(false), m_length(0), m_ended(false) {}

  void read(int ch) {
    if (ch == ',') {
      finish();
      m_length = 0;
      m_ended = false;
    } else if (ch == ' ' || ch == '\t') {
      if (m_length) m_ended = true;
    } else if (m_ended || m_length >= sizeof(m_token) - 1) {
      m_length = sizeof(m_token);
    } else {
      m_token[m_length++] = (char)ch;
    }
  }

  void finish() {
    if (!m_length || m_length >= sizeof(m_token)) return;
    m_token[m_length] = '\0';
    P(closeToken) = "close";
    P(keepAliveToken) = "keep-alive";
    close |= App::strcmpiP(m_token, closeToken) == 0;
    keepAlive |= App::strcmpiP(m_token, keepAliveToken) == 0;
  }

  bool close;
  bool keepAlive;

 private:
  char m_token[11];
  uint8_t m_length;
  bool m_ended;
};

}
}

#endif
