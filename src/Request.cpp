#include "Request.h"

#include <limits.h>

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
  return min(m_stream->available(), m_left) + m_pushbackDepth;
}

int Request::bytesRead() { return m_bytesRead; }

Client* Request::client() { return m_stream; }

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

namespace {
int hexDigit(int ch) {
  if (ch >= '0' && ch <= '9') return ch - '0';
  if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  return -1;
}

// The caller identifies component boundaries before decoding.
int decodedChar(const char*& cursor, const char* end, bool plusAsSpace) {
  int ch = (unsigned char)*cursor++;
  if (ch == '%' && end - cursor >= 2) {
    int high = hexDigit(cursor[0]);
    int low = hexDigit(cursor[1]);
    cursor += 2;
    return high < 0 || low < 0 ? -1 : (high << 4) | low;
  }
  if (ch == '%') return -1;
  return ch == '+' && plusAsSpace ? ' ' : ch;
}
}  // namespace

bool Request::form(char* name, int nameLength, char* value, int valueLength) {
  if (!name || !value || nameLength <= 0 || valueLength <= 0) return false;
  *name = *value = '\0';
  bool foundSomething = false;
  bool readingName = true;
  bool valid = true;
  int ch;

  while ((ch = m_timedRead()) != -1) {
    foundSomething = true;
    if (ch == '&') break;
    if (ch == '=' && readingName) {
      readingName = false;
      continue;
    }
    if (ch == '+') {
      ch = ' ';
    } else if (ch == '%') {
      int highByte = m_timedRead();
      if (highByte == -1 || highByte == '&') return false;
      int lowByte = m_timedRead();
      if (lowByte == -1 || lowByte == '&') return false;
      int high = hexDigit(highByte);
      int low = hexDigit(lowByte);
      if (high < 0 || low < 0) {
        valid = false;
        continue;
      }
      ch = (high << 4) | low;
    }
    if (ch == 0) valid = false;

    char*& output = readingName ? name : value;
    int& remaining = readingName ? nameLength : valueLength;
    if (remaining <= 1) valid = false;
    if (valid) {
      *output++ = (char)ch;
      *output = '\0';
      --remaining;
    }
  }
  return foundSomething && valid && (ch == '&' || left() == 0);
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
  // Keep the most recently pushed byte when the stack is full.
  if (m_pushbackDepth == SERVER_PUSHBACK_BUFFER_SIZE) --m_pushbackDepth;
  m_pushback[m_pushbackDepth++] = ch;
}

char* Request::query() { return m_query; }

bool Request::query(const char* name, char* buffer, int bufferLength) {
  if (!buffer || bufferLength <= 0 || !name) return false;
  *buffer = '\0';
  if (!m_query) return false;

  const char* field = m_query;
  while (*field) {
    const char* end = strchr(field, '&');
    if (!end) end = field + strlen(field);
    const char* equals = field;
    while (equals < end && *equals != '=') ++equals;
    const char* cursor = field;
    const char* candidate = name;
    bool matches = true;
    while (cursor < equals) {
      int ch = decodedChar(cursor, equals, true);
      if (ch <= 0 || !*candidate || ch != (unsigned char)*candidate) {
        matches = false;
        break;
      }
      ++candidate;
    }
    if (matches && !*candidate && equals < end) {
      cursor = equals + 1;
      while (cursor < end) {
        int ch = decodedChar(cursor, end, true);
        if (ch <= 0 || bufferLength <= 1) return false;
        *buffer++ = (char)ch;
        *buffer = '\0';
        --bufferLength;
      }
      return true;
    }
    field = *end ? end + 1 : end;
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

  while (m_pushbackDepth > 0 && size > 0) {
    *buf++ = m_pushback[--m_pushbackDepth];
    size--;
    ret++;
  }

  if (size == 0 || m_left == 0) return ret;
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
  if (!m_pattern || !*m_pattern || !name || !buffer || bufferLength <= 0) {
    return false;
  }
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
  if (!m_route || !buffer || bufferLength <= 0) return false;
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

  // Try each verb in sequence, but stop as soon as a read has timed out —
  // otherwise a client that connects and never sends data costs us one
  // full Stream::_timeout per verb (seven of them).
  if (m_expectP(GET_VERB)) {
    m_method = GET;
    return true;
  }
  if (m_readTimedout) return false;
  if (m_expectP(HEAD_VERB)) {
    m_method = HEAD;
    return true;
  }
  if (m_readTimedout) return false;
  if (m_expectP(POST_VERB)) {
    m_method = POST;
    return true;
  }
  if (m_readTimedout) return false;
  if (m_expectP(PUT_VERB)) {
    m_method = PUT;
    return true;
  }
  if (m_readTimedout) return false;
  if (m_expectP(DELETE_VERB)) {
    m_method = DELETE;
    return true;
  }
  if (m_readTimedout) return false;
  if (m_expectP(PATCH_VERB)) {
    m_method = PATCH;
    return true;
  }
  if (m_readTimedout) return false;
  if (m_expectP(OPTIONS_VERB)) {
    m_method = OPTIONS;
    return true;
  }
  return false;
}

bool Request::m_readURL() {
  char* output = m_path;
  int remaining = m_pathLength;
  int ch;
  *output = '\0';
  while ((ch = m_timedRead()) != -1) {
    if (ch == ' ') return output != m_path;
    if (ch <= 0x20 || ch == 0x7f || remaining == 0) return false;
    *output++ = (char)ch;
    *output = '\0';
    --remaining;
  }
  return false;
}

bool Request::m_readVersion() {
  P(HTTP_VERSION) = "HTTP/1.";
  for (size_t i = 0; pgm_read_byte(HTTP_VERSION + i); ++i) {
    if (m_timedRead() != pgm_read_byte(HTTP_VERSION + i)) return false;
  }
  int minor = m_timedRead();
  if ((minor != '0' && minor != '1') || !m_expect(CRLF)) return false;
  m_minorVersion = minor - '0';
  return true;
}

bool Request::m_processURL() {
  // Validate the entire raw target, including query components not requested by
  // the handler, before modifying the path in place.
  const char* cursor = m_path;
  const char* end = m_path + strlen(m_path);
  while (cursor < end) {
    if (decodedChar(cursor, end, false) <= 0) return false;
  }
  char* separator = strchr(m_path, '?');
  m_query = separator ? separator + 1 : m_path + strlen(m_path);
  if (separator) *separator = '\0';
  end = separator ? separator : m_query;
  cursor = m_path;
  char* output = m_path;
  while (cursor < end) *output++ = (char)decodedChar(cursor, end, false);
  *output = '\0';
  return true;
}

int Request::m_processHeaders() {
  bool canEnd = true;
  bool lengthSeen = false;

  while (!(canEnd && m_expect(CRLF))) {
    canEnd = false;
    P(ContentLength) = "Content-Length:";
    if (m_expectP(ContentLength)) {
      int length;
      if (!m_readInt(length) || !m_skipSpace() || !m_expect(CRLF) ||
          (lengthSeen && length != m_left)) {
        return 400;
      }
      m_left = length;
      lengthSeen = true;
      canEnd = true;
    } else {
      P(TransferEncoding) = "Transfer-Encoding:";
      // Request transfer codings are unsupported, including chunked.
      if (m_expectP(TransferEncoding)) return 400;
      HeaderNode* headerNode;
      if (!m_headerName(headerNode)) return 400;
      if (headerNode) {
        if (!m_headerValue(headerNode->buffer, headerNode->bufferLength)) {
          return 431;
        }
        canEnd = true;
      }
    }

    if (!canEnd) {
      while (!m_expect(CRLF)) {
        if (m_timedRead() == -1) {
          return 431;
        }
      }

      canEnd = true;
    }
  }

  m_readingContent = true;

  return 0;
}

bool Request::m_headerName(HeaderNode*& header) {
  header = m_headerTail;
  size_t matched = 0;
  bool hasName = false;
  int ch;
  while ((ch = m_timedRead()) != -1) {
    if (ch != ':') {
      bool token = (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') ||
                   (ch >= 'a' && ch <= 'z') ||
                   (ch > 0 && strchr("!#$%&'*+-.^_`|~", ch));
      if (!token) return false;
      hasName = true;
    }

    int expected = ch == ':' ? 0 : tolower(ch);
    if (header && tolower((unsigned char)header->name[matched]) != expected) {
      // The current candidate remembers the prefix already read. Find the
      // next candidate with that prefix and this character without rewinding
      // the stream or storing a copy of an arbitrarily long field name.
      HeaderNode* candidate = header->next;
      while (candidate) {
        size_t i = 0;
        while (i < matched && candidate->name[i] &&
               tolower((unsigned char)candidate->name[i]) ==
                   tolower((unsigned char)header->name[i])) {
          ++i;
        }
        if (i == matched &&
            tolower((unsigned char)candidate->name[i]) == expected) {
          break;
        }
        candidate = candidate->next;
      }
      header = candidate;
    }
    if (ch == ':') return hasName;
    if (header) ++matched;
  }
  return false;
}

bool Request::m_headerValue(char* buffer, int bufferLength) {
  if (!buffer || bufferLength <= 0) return false;
  if (*buffer) {
    int length = strlen(buffer);
    if (length + 1 >= bufferLength) return false;
    buffer[length] = ',';
    buffer += length + 1;
    bufferLength -= length + 1;
  }
  *buffer = '\0';
  if (!m_skipSpace()) return false;
  bool valid = true;
  while (!m_expect(CRLF)) {
    int ch = m_timedRead();
    if (ch < 0) return false;
    if (ch == 0 || ch == '\r' || ch == '\n') valid = false;
    if (bufferLength <= 1) valid = false;
    if (valid) {
      *buffer++ = (char)ch;
      *buffer = '\0';
      --bufferLength;
    }
  }
  return valid;
}

bool Request::m_readInt(int& number) {
  if (!m_skipSpace()) return false;
  bool gotNumber = false;
  number = 0;
  int ch;
  while ((ch = m_timedRead()) >= '0' && ch <= '9') {
    int digit = ch - '0';
    if (number > (INT_MAX - digit) / 10) return false;
    number = number * 10 + digit;
    gotNumber = true;
  }
  if (ch < 0) return false;
  push(ch);
  return gotNumber;
}

void Request::m_setRoute(const char* route, const char* pattern) {
  m_route = route;
  m_pattern = pattern;
}

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

bool Request::m_timedout() { return m_readTimedout; }

int Request::m_timedRead() {
  // Avoid consulting the clock while data is already available.
  int ch = read();
  if (ch == -1 && _timeout != 0) ch = timedRead();
  if (ch == -1) {
    m_readTimedout = true;
  }

  return ch;
}
