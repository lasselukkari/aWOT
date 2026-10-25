#include "Request.h"

#include <limits.h>

#include "App.h"

using namespace awot;

Request::Request(Client* client, Response* m_response,
                 ArrayView<Header> headers, char* urlBuffer,
                 int urlBufferLength, unsigned long timeout, void* context)
    : context(context),
      m_stream(client),
      m_response(m_response),
      m_method(UNKNOWN),
      m_minorVersion(-1),
      m_connectionClose(false),
      m_connectionKeepAlive(false),
      m_pushback(),
      m_pushbackDepth(0),
      m_readingContent(false),
      m_bodyState(CONTENT_LENGTH),
      m_left(0),
      m_bytesRead(0),
      m_headers(headers),
      m_query(NULL),
      m_readTimedout(false),
      m_path(urlBuffer),
      m_pathLength(urlBufferLength - 1),
      m_pattern(NULL),
      m_route(NULL),
      m_next(true) {
  *m_path = '\0';
  _timeout = timeout;
}

int Request::availableForWrite() { return m_response->availableForWrite(); }

int Request::available() {
  if (m_readingContent && !m_bodyAvailable()) return m_pushbackDepth;
  int count = (int)min((uint32_t)m_stream->available(), m_left);
  return count + min(m_pushbackDepth, INT_MAX - count);
}

int Request::bytesRead() { return m_bytesRead; }

Client* Request::client() { return m_stream; }

char* Request::get(const char* name) {
  for (int i = 0; i < m_headers.size(); ++i) {
    Header& header = m_headers[i];
    if (header.m_present && App::strcmpi(header.name, name) == 0) {
      return header.buffer.data();
    }
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

bool headerToken(int ch) {
  return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') ||
         (ch >= 'a' && ch <= 'z') || (ch > 0 && strchr("!#$%&'*+-.^_`|~", ch));
}

bool headerValueChar(int ch) {
  return ch == '\t' || (ch >= 0x20 && ch != 0x7f);
}

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

int Request::left() {
  if (m_readingContent && m_bodyState != CONTENT_LENGTH) {
    m_bodyAvailable();
    if (m_bodyState != BODY_DONE) return -1;
  }
  return (int)m_left + m_pushbackDepth;
}

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

  if (m_readingContent && !m_bodyAvailable()) return -1;

  int ch = m_readRaw();
  if (ch == -1) {
    return -1;
  }

  if (m_readingContent) {
    m_left--;
  }

  return ch;
}

int Request::read(uint8_t* buf, size_t size) {
  int ret = 0;

  while (m_pushbackDepth > 0 && size > 0) {
    *buf++ = m_pushback[--m_pushbackDepth];
    size--;
    ret++;
  }

  while (size > 0 && m_bodyAvailable()) {
    size_t count = min(size, (size_t)min(m_left, (uint32_t)(INT_MAX - ret)));
    if (!count) break;
    int read = m_stream->read(buf, count);
    if (read < 0) return ret ? ret : -1;
    if (!read) break;
    ret += read;
    buf += read;
    size -= read;
    m_bytesRead += min(read, INT_MAX - m_bytesRead);
    m_left -= read;
    if (m_bodyState == CONTENT_LENGTH) break;
  }
  return ret;
}

int Request::m_readRaw() {
  int ch = m_stream->read();
  if (ch >= 0 && m_bytesRead < INT_MAX) ++m_bytesRead;
  return ch;
}

bool Request::m_bodyAvailable() {
  if (m_bodyState == CONTENT_LENGTH) {
    if (!m_left) _timeout = 0;
    return m_left > 0;
  }

  for (;;) {
    if (m_bodyState == CHUNK_DATA) {
      if (m_left) return true;
      m_bodyState = CHUNK_DATA_CR;
    }
    if (m_bodyState == BODY_DONE || m_bodyState == BODY_ERROR) {
      _timeout = 0;
      return false;
    }
    int ch = m_readRaw();
    if (ch < 0) return false;

    switch (m_bodyState) {
      case CHUNK_START: {
        int digit = hexDigit(ch);
        if (digit < 0) break;
        m_left = digit;
        m_bodyState = CHUNK_SIZE;
        continue;
      }
      case CHUNK_SIZE: {
        int digit = hexDigit(ch);
        if (digit >= 0) {
          if (m_left > UINT32_MAX / 16) break;
          m_left = m_left * 16 + digit;
        } else if (ch == ';') {
          m_bodyState = CHUNK_EXTENSION;
        } else if (ch == ' ' || ch == '\t') {
          m_bodyState = CHUNK_EXTENSION_START;
        } else if (ch == '\r') {
          m_bodyState = CHUNK_SIZE_LF;
        } else {
          break;
        }
        continue;
      }
      case CHUNK_EXTENSION_START:
        if (ch == ' ' || ch == '\t') continue;
        if (ch != ';') break;
        m_bodyState = CHUNK_EXTENSION;
        continue;
      case CHUNK_EXTENSION:
        if (ch == '\r') {
          m_bodyState = CHUNK_SIZE_LF;
        } else if (!headerValueChar(ch)) {
          break;
        }
        continue;
      case CHUNK_SIZE_LF:
        if (ch != '\n') break;
        m_bodyState = m_left ? CHUNK_DATA : TRAILER_START;
        continue;
      case CHUNK_DATA_CR:
        if (ch != '\r') break;
        m_bodyState = CHUNK_DATA_LF;
        continue;
      case CHUNK_DATA_LF:
        if (ch != '\n') break;
        m_bodyState = CHUNK_START;
        continue;
      case TRAILER_START:
        if (ch == '\r') {
          m_bodyState = BODY_END_LF;
        } else if (headerToken(ch)) {
          m_bodyState = TRAILER_NAME;
        } else {
          break;
        }
        continue;
      case TRAILER_NAME:
        if (ch == ':') {
          m_bodyState = TRAILER_VALUE;
        } else if (!headerToken(ch)) {
          break;
        }
        continue;
      case TRAILER_VALUE:
        if (ch == '\r') {
          m_bodyState = TRAILER_LF;
        } else if (!headerValueChar(ch)) {
          break;
        }
        continue;
      case TRAILER_LF:
        if (ch != '\n') break;
        m_bodyState = TRAILER_START;
        continue;
      case BODY_END_LF:
        if (ch != '\n') break;
        m_bodyState = BODY_DONE;
        continue;
      default:
        break;
    }
    m_bodyState = BODY_ERROR;
    m_left = 0;
  }
}

int Request::m_bodyError() const {
  if (m_bodyState == BODY_ERROR) return 400;
  if (m_bodyState != CONTENT_LENGTH && m_readTimedout) return 408;
  return 0;
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

bool Request::m_allowsKeepAlive() const {
  if (m_connectionClose) return false;
  return m_minorVersion == 1 || (m_minorVersion == 0 && m_connectionKeepAlive);
}

void Request::next() { m_next = true; }

size_t Request::write(uint8_t data) { return m_response->write(data); }

size_t Request::write(const uint8_t* buffer, size_t bufferLength) {
  return m_response->write(buffer, bufferLength);
}

bool Request::m_processMethod() {
  // Keep verbs in MethodType order (GET through OPTIONS).
  // Those enum values must remain consecutive.
  P(VERBS) = "GET \0HEAD \0POST \0PUT \0DELETE \0PATCH \0OPTIONS ";
  const unsigned char* verb = VERBS;

  for (int method = GET; method <= OPTIONS; ++method) {
    if (m_expectP(verb)) {
      m_method = static_cast<MethodType>(method);
      return true;
    }
    if (m_readTimedout) return false;

    while (pgm_read_byte(verb++)) {
    }
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
      if (m_bodyState != CONTENT_LENGTH || !m_readInt(length) ||
          !m_skipSpace() || !m_expect(CRLF) ||
          (lengthSeen && (uint32_t)length != m_left)) {
        return 400;
      }
      m_left = length;
      lengthSeen = true;
      canEnd = true;
    } else {
      P(TransferEncoding) = "Transfer-Encoding:";
      if (m_expectP(TransferEncoding)) {
        P(chunked) = "chunked";
        if (lengthSeen || m_bodyState != CONTENT_LENGTH ||
            m_minorVersion != 1 || !m_skipSpace() || !m_expectP(chunked) ||
            !m_skipSpace() || !m_expect(CRLF)) {
          return 400;
        }
        m_bodyState = CHUNK_START;
        canEnd = true;
        continue;
      }
      Header* header;
      bool connection;
      if (!m_headerName(header, connection)) return 400;
      if (!m_headerValue(header, connection)) return header ? 431 : 400;
      if (header) header->m_present = true;
      canEnd = true;
    }
  }

  P(lengthName) = "Content-Length";
  P(encodingName) = "Transfer-Encoding";
  for (int i = 0; i < m_headers.size(); ++i) {
    Header& header = m_headers[i];
    char length[3 * sizeof(int) + 1];
    const char* value;
    if (lengthSeen && App::strcmpiP(header.name, lengthName) == 0) {
      value = itoa((int)m_left, length, 10);
    } else if (m_bodyState == CHUNK_START &&
               App::strcmpiP(header.name, encodingName) == 0) {
      value = "chunked";
    } else {
      continue;
    }
    size_t size = strlen(value) + 1;
    if (size > (size_t)header.buffer.size()) return 431;
    memcpy(header.buffer.data(), value, size);
    header.m_present = true;
  }

  m_readingContent = true;

  return 0;
}

bool Request::m_headerName(Header*& header, bool& connection) {
  int index = 0;
  header = m_headers.size() > 0 ? &m_headers[0] : nullptr;
  size_t matched = 0;
  size_t length = 0;
  bool hasName = false;
  connection = true;
  P(connectionName) = "Connection";
  int ch;
  while ((ch = m_timedRead()) != -1) {
    if (ch != ':') {
      if (!headerToken(ch)) return false;
      hasName = true;
    }

    int expected = ch == ':' ? 0 : tolower(ch);
    if (connection &&
        tolower(pgm_read_byte(connectionName + length)) != expected) {
      connection = false;
    }
    if (header && tolower((unsigned char)header->name[matched]) != expected) {
      Header* candidate = nullptr;
      while (++index < m_headers.size()) {
        candidate = &m_headers[index];
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
        candidate = nullptr;
      }
      header = candidate;
    }
    if (ch == ':') return hasName;
    if (header) ++matched;
    ++length;
  }
  return false;
}

bool Request::m_headerValue(Header* header, bool connection) {
  char* buffer = NULL;
  int bufferLength = 0;
  if (header) {
    buffer = header->buffer.data();
    bufferLength = header->buffer.size();
  }
  if (header && header->m_present) {
    int length = strlen(buffer);
    if (length + 1 >= bufferLength) return false;
    buffer[length] = ',';
    buffer += length + 1;
    bufferLength -= length + 1;
  }
  if (header) *buffer = '\0';
  if (!m_skipSpace()) return false;

  bool valid = true;
  ConnectionParser connectionValue;
  while (!m_expect(CRLF)) {
    int ch = m_timedRead();
    if (ch < 0) return false;
    if (!headerValueChar(ch)) return false;

    if (connection) connectionValue.read(ch);

    if (header && bufferLength > 1 && valid) {
      *buffer++ = (char)ch;
      *buffer = '\0';
      --bufferLength;
    } else if (header) {
      valid = false;
    }
  }

  if (connection) {
    connectionValue.finish();
    m_connectionClose |= connectionValue.close;
    m_connectionKeepAlive |= connectionValue.keepAlive;
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
  if (m_readTimedout) return -1;

  int ch = read();
  if (ch == -1 && _timeout != 0) ch = timedRead();
  if (ch == -1) {
    if (m_readingContent &&
        (m_bodyState == BODY_DONE || m_bodyState == BODY_ERROR ||
         (m_bodyState == CONTENT_LENGTH && !m_left))) {
      return -1;
    }
    m_readTimedout = true;
  }

  return ch;
}
