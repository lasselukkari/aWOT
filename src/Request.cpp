/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things
*/

#include "Request.h"

#include "App.h"

using namespace awot;

Request::Request(Client* client, Response* m_response, HeaderNode* headerTail,
                 char* urlBuffer, int urlBufferLength, unsigned long timeout,
                 void* context)
    : context(context),
      m_stream(client),
      m_response(m_response),
      m_method(UNKNOWN),
      m_minorVersion(-1),
      m_pushback(),
      m_pushbackDepth(0),
      m_readingContent(false),
      m_left(0),
      m_bytesRead(0),
      m_headerTail(headerTail),
      m_query(NULL),
      m_queryLength(0),
      m_readTimedout(false),
      m_path(urlBuffer),
      m_pathLength(urlBufferLength - 1),
      m_pattern(NULL),
      m_route(NULL),
      m_next(true) {
  _timeout = timeout;
}

int Request::availableForWrite() { return m_response->availableForWrite(); }

int Request::available() {
  return min(m_stream->available(), m_left + m_pushbackDepth);
}

int Request::bytesRead() { return m_bytesRead; }

Stream* Request::stream() { return m_stream; }

char* Request::get(const char* name) {
  HeaderNode* headerNode = m_headerTail;

  while (headerNode != NULL) {
    if (App::strcmpi(headerNode->name, name) == 0) {
      return headerNode->buffer;
    }

    headerNode = headerNode->next;
  }

  return NULL;
}

void Request::flush() { return m_response->flush(); }

bool Request::form(char* name, int nameLength, char* value, int valueLength) {
  int ch;
  bool foundSomething = false;
  bool readingName = true;

  memset(name, 0, nameLength);
  memset(value, 0, valueLength);

  while ((ch = m_timedRead()) != -1) {
    foundSomething = true;
    if (ch == '+') {
      ch = ' ';
    } else if (ch == '=') {
      readingName = false;
      continue;
    } else if (ch == '&') {
      return nameLength > 0 && valueLength > 0;
    } else if (ch == '%') {
      int high = m_timedRead();
      if (high == -1) {
        return false;
      }

      int low = m_timedRead();
      if (low == -1) {
        return false;
      }

      if (high > 0x39) {
        high -= 7;
      }

      high &= 0x0f;

      if (low > 0x39) {
        low -= 7;
      }

      low &= 0x0f;

      ch = (high << 4) | low;
    }

    if (readingName && --nameLength) {
      *name++ = ch;
    } else if (!readingName && --valueLength) {
      *value++ = ch;
    }
  }

  return foundSomething && nameLength > 0 && valueLength > 0;
}

int Request::left() { return m_left + m_pushbackDepth; }

Request::MethodType Request::method() { return m_method; }

char* Request::path() { return m_path; }

int Request::peek() {
  int ch = read();

  if (ch != -1) {
    push(ch);
  }

  return ch;
}

void Request::push(uint8_t ch) {
  m_pushback[m_pushbackDepth++] = ch;

  // can't raise error here, so just replace last char over and over
  if (m_pushbackDepth == SERVER_PUSHBACK_BUFFER_SIZE) {
    m_pushbackDepth = SERVER_PUSHBACK_BUFFER_SIZE - 1;
  }
}

char* Request::query() { return m_query; }

bool Request::query(const char* name, char* buffer, int bufferLength) {
  memset(buffer, 0, bufferLength);

  char* position = m_query;
  int nameLength = strlen(name);

  while ((position = strstr(position, name))) {
    char previous = *(position - 1);

    if ((previous == '\0' || previous == '&') &&
        *(position + nameLength) == '=') {
      position = position + nameLength + 1;
      while (*position && *position != '&' && --bufferLength) {
        *buffer++ = *position++;
      }

      return bufferLength > 0;
    }

    position++;
  }

  return false;
}

int Request::read() {
  if (m_pushbackDepth > 0) {
    return m_pushback[--m_pushbackDepth];
  }

  if (m_readingContent && !m_left) {
    _timeout = 0;
    return -1;
  }

  int ch = m_stream->read();
  if (ch == -1) {
    return -1;
  }

  if (m_readingContent) {
    m_left--;
  }

  m_bytesRead++;
  return ch;
}

int Request::read(uint8_t* buf, size_t size) {
  int ret = 0;

  while (m_pushbackDepth > 0) {
    *buf++ = m_pushback[--m_pushbackDepth];
    size--;
    ret++;
  }

  int read = m_stream->read(buf, (size < (unsigned)m_left ? size : m_left));
  if (read == -1) {
    if (ret > 0) {
      return ret;
    }

    return -1;
  }

  ret += read;
  m_bytesRead += read;
  m_left -= read;

  return ret;
}

bool Request::route(const char* name, char* buffer, int bufferLength) {
  int part = 0;
  int i = 1;

  while (m_pattern[i]) {
    if (m_pattern[i] == '/') {
      part++;
    }

    if (m_pattern[i++] == ':') {
      int j = 0;

      while ((m_pattern[i] && name[j]) && m_pattern[i] == name[j]) {
        i++;
        j++;
      }

      if (!name[j] && (m_pattern[i] == '/' || !m_pattern[i])) {
        return route(part, buffer, bufferLength);
      }
    }
  }

  return false;
}

bool Request::route(int number, char* buffer, int bufferLength) {
  memset(buffer, 0, bufferLength);
  int part = -1;
  const char* routeStart = m_route;

  while (*routeStart) {
    if (*routeStart++ == '/') {
      part++;

      if (part == number) {
        while (*routeStart && *routeStart != '/' && --bufferLength) {
          *buffer++ = *routeStart++;
        }

        return bufferLength > 0;
      }
    }
  }

  return false;
}

int Request::minorVersion() { return m_minorVersion; }

void Request::next() { m_next = true; }

size_t Request::write(uint8_t data) { return m_response->write(data); }

size_t Request::write(const uint8_t* buffer, size_t bufferLength) {
  return m_response->write(buffer, bufferLength);
}

bool Request::m_processMethod() {
  P(GET_VERB) = "GET ";
  P(HEAD_VERB) = "HEAD ";
  P(POST_VERB) = "POST ";
  P(PUT_VERB) = "PUT ";
  P(DELETE_VERB) = "DELETE ";
  P(PATCH_VERB) = "PATCH ";
  P(OPTIONS_VERB) = "OPTIONS ";

  if (m_expectP(GET_VERB)) {
    m_method = GET;
  } else if (m_expectP(HEAD_VERB)) {
    m_method = HEAD;
  } else if (m_expectP(POST_VERB)) {
    m_method = POST;
  } else if (m_expectP(PUT_VERB)) {
    m_method = PUT;
  } else if (m_expectP(DELETE_VERB)) {
    m_method = DELETE;
  } else if (m_expectP(PATCH_VERB)) {
    m_method = PATCH;
  } else if (m_expectP(OPTIONS_VERB)) {
    m_method = OPTIONS;
  } else {
    return false;
  }

  return true;
}

bool Request::m_readURL() {
  char* request = m_path;
  int bufferLeft = m_pathLength;
  int ch;

  while ((ch = m_timedRead()) != -1 && ch != ' ' && ch != '\n' && ch != '\r' &&
         --bufferLeft) {
    if (ch == '%') {
      int high = m_timedRead();
      if (high == -1) {
        return false;
      }

      int low = m_timedRead();
      if (low == -1) {
        return false;
      }

      if (high > 0x39) {
        high -= 7;
      }

      high &= 0x0f;

      if (low > 0x39) {
        low -= 7;
      }

      low &= 0x0f;

      ch = (high << 4) | low;
    }

    *request++ = ch;
  }

  *request = 0;

  return bufferLeft > 0;
}

bool Request::m_readVersion() {
  while (!m_expect(CRLF)) {
    P(HTTP_10) = "1.0";
    P(HTTP_11) = "1.1";

    if (m_expectP(HTTP_10)) {
      m_minorVersion = 0;
    } else if (m_expectP(HTTP_11)) {
      m_minorVersion = 1;
    } else if (m_timedRead() == -1) {
      return false;
    }
  }

  return true;
}

void Request::m_processURL() {
  char* qmLocation = strchr(m_path, '?');
  int qmOffset = (qmLocation == NULL) ? 0 : 1;

  m_pathLength = (qmLocation == NULL) ? strlen(m_path) : (qmLocation - m_path);
  m_query = m_path + m_pathLength + qmOffset;
  m_queryLength = strlen(m_query);

  if (qmOffset) {
    *qmLocation = 0;
  }
}

bool Request::m_processHeaders() {
  bool canEnd = true;

  while (!(canEnd && m_expect(CRLF))) {
    canEnd = false;
    P(ContentLength) = "Content-Length:";
    if (m_expectP(ContentLength)) {
      if (!m_readInt(m_left) || !m_expect(CRLF)) {
        return false;
      }

      canEnd = true;
    } else {
      HeaderNode* headerNode = m_headerTail;

      while (headerNode != NULL) {
        P(headerSeparator) = ":";
        if (m_expect(headerNode->name) && m_expectP(headerSeparator)) {
          if (!m_headerValue(headerNode->buffer, headerNode->bufferLength)) {
            return false;
          }

          canEnd = true;
          break;
        }

        headerNode = headerNode->next;
      }
    }

    if (!canEnd) {
      while (!m_expect(CRLF)) {
        if (m_timedRead() == -1) {
          return false;
        }
      }

      canEnd = true;
    }
  }

  m_readingContent = true;

  return true;
}

bool Request::m_headerValue(char* buffer, int bufferLength) {
  int ch;

  if (buffer[0] != '\0') {
    int length = strlen(buffer);
    buffer[length] = ',';
    buffer = buffer + length + 1;
    bufferLength = bufferLength - (length + 1);
  }

  if (!m_skipSpace()) {
    return false;
  }

  while ((ch = m_timedRead()) != -1) {
    if (--bufferLength > 0) {
      *buffer++ = ch;
    }

    if (m_expect(CRLF)) {
      *buffer = '\0';
      return bufferLength > 0;
    }
  }

  return false;
}

bool Request::m_readInt(int& number) {
  bool negate = false;
  bool gotNumber = false;

  if (!m_skipSpace()) {
    return false;
  }

  int ch = m_timedRead();
  if (ch == -1) {
    return false;
  }

  if (ch == '-') {
    negate = true;
    ch = m_timedRead();
    if (ch == -1) {
      return false;
    }
  }

  number = 0;

  while (ch >= '0' && ch <= '9') {
    gotNumber = true;
    number = number * 10 + ch - '0';
    ch = m_timedRead();
    if (ch == -1) {
      return false;
    }
  }

  push(ch);

  if (negate) {
    number = -number;
  }

  return gotNumber;
}

void Request::m_setRoute(const char* route, const char* pattern) {
  m_route = route;
  m_pattern = pattern;
}

int Request::m_getUrlPathLength() { return m_pathLength; }

bool Request::m_expect(const char* expected) {
  const char* candidate = expected;

  while (*candidate != 0) {
    int ch = m_timedRead();
    if (ch == -1) {
      return false;
    }

    if (tolower(ch) != tolower(*candidate++)) {
      push(ch);

      while (--candidate != expected) {
        push(candidate[-1]);
      }

      return false;
    }
  }

  return true;
}

bool Request::m_expectP(const unsigned char* expected) {
  const unsigned char* candidate = expected;

  while (pgm_read_byte(candidate) != 0) {
    int ch = m_timedRead();
    if (ch == -1) {
      return false;
    }

    if (tolower(ch) != tolower(pgm_read_byte(candidate++))) {
      push(ch);

      while (--candidate != expected) {
        push(pgm_read_byte(candidate - 1));
      }

      return false;
    }
  }

  return true;
}

bool Request::m_skipSpace() {
  int ch;

  while ((ch = m_timedRead()) != -1 && (ch == ' ' || ch == '\t'));

  if (ch == -1) {
    return false;
  }

  push(ch);

  return true;
}

void Request::m_reset() {
  HeaderNode* headerNode = m_headerTail;
  while (headerNode != NULL) {
    headerNode->buffer[0] = '\0';
    headerNode = headerNode->next;
  }
}

bool Request::m_timedout() { return m_readTimedout; }

int Request::m_timedRead() {
  int ch = timedRead();
  if (ch == -1) {
    m_readTimedout = true;
  }

  return ch;
}
