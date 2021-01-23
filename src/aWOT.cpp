/*
  aWOT, Express.js inspired microcontreller web framework for the Web of Things

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "aWOT.h"

Response::Response()
    : m_stream(NULL),
      m_headers(),
      m_contentLenghtSet(false),
      m_contentTypeSet(false),
      m_keepAlive(false),
      m_statusSent(0),
      m_headersSent(false),
      m_sendingStatus(false),
      m_sendingHeaders(false),
      m_headersCount(0),
      m_mime(NULL),
      m_bytesSent(0),
      m_ended(false),
      m_buffer(),
      m_bufFill(0) {}

int Response::availableForWrite() {
  return SERVER_OUTPUT_BUFFER_SIZE - m_bufFill - 1;
}

void Response::beginHeaders() {
  if (!m_statusSent) {
    status(200);
  }

  m_sendingHeaders = true;

  for (int i = 0; i < m_headersCount; i++) {
    print(m_headers[i].name);
    print(": ");
    print(m_headers[i].value);
    m_printCRLF();
  }
}

int Response::bytesSent() { return m_bytesSent; }

void Response::end() {
  m_ended = true;
}

void Response::endHeaders() {
  m_printCRLF();
  m_flushBuf();
  m_sendingHeaders = false;
  m_headersSent = true;
}

bool Response::ended() { return m_ended; }

void Response::flush() {
  m_flushBuf();

  m_stream->flush();
}

const char *Response::get(const char *name) {
  for (int i = 0; i < m_headersCount; i++) {
    if (Application::strcmpi(name, m_headers[i].name) == 0) {
      return m_headers[m_headersCount].value;
    }
  }

  return NULL;
}

bool Response::headersSent() { return m_headersSent; }

void Response::printP(const unsigned char *string) {
  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  while (uint8_t value = pgm_read_byte(string++)) {
    write(value);
  }
}

void Response::printP(const char *string) { printP((unsigned char *)string); }

void Response::sendStatus(int code) {
  status(code);

  m_printHeaders();

  if (code != 204 && code != 304) {
    m_printStatus(code);
  }
}

void Response::set(const char *name, const char *value) {
  if (m_headersCount >= SIZE(m_headers)) {
    return;
  }

  m_headers[m_headersCount].name = name;
  m_headers[m_headersCount].value = value;
  m_headersCount++;

  if (Application::strcmpi(name, "Content-Type") == 0) {
    m_contentTypeSet = true;
  }

  if (Application::strcmpi(name, "Content-Length") == 0) {
    m_contentLenghtSet = true;
  }

  if (Application::strcmpi(name, "Connection") == 0) {
    m_keepAlive = Application::strcmpi(value, "keep-alive") == 0;
  }
}

void Response::setDefaults() {
  if (!m_contentTypeSet) {
    set("Content-Type", "text/plain");
  }

  if (m_keepAlive && !m_contentLenghtSet) {
    set("Transfer-Encoding", "chunked");
  }

  if (!m_keepAlive) {
    m_contentLenghtSet = true;
    set("Connection", "close");
  }
}

void Response::status(int code) {
  if (m_statusSent) {
    return;
  }

  m_statusSent = code;

  m_sendingStatus = true;
  P(httpVersion) = "HTTP/1.1 ";
  printP(httpVersion);
  print(code);
  print(" ");
  m_printStatus(code);

  m_printCRLF();

  if (code < 200) {
    beginHeaders();
    endHeaders();
    m_statusSent = 0;
  } else if (code == 204 || code == 304) {
    m_contentLenghtSet = true;
    m_contentTypeSet = true;
  }

  m_sendingStatus = false;
}

int Response::statusSent() { return m_statusSent; }

size_t Response::write(uint8_t data) {
  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  m_buffer[m_bufFill++] = data;

  if (m_bufFill == SERVER_OUTPUT_BUFFER_SIZE) {
    if (m_headersSent && !m_contentLenghtSet) {
      m_stream->print(m_bufFill, HEX);
      m_stream->print(CRLF);
    }

    m_stream->write(m_buffer, SERVER_OUTPUT_BUFFER_SIZE);

    if (m_headersSent && !m_contentLenghtSet) {
      m_stream->print(CRLF);
    }

    m_bufFill = 0;
  }

  size_t bytesSent = sizeof(data);
  m_bytesSent += bytesSent;
  return bytesSent;
}

size_t Response::write(uint8_t *buffer, size_t bufferLength) {
  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  m_flushBuf();

  if (m_headersSent && !m_contentLenghtSet) {
    m_stream->print(bufferLength, HEX);
    m_stream->print(CRLF);
  }

  m_stream->write(buffer, bufferLength);

  if (m_headersSent && !m_contentLenghtSet) {
    m_stream->print(CRLF);
  }

  m_bytesSent += bufferLength;
  return bufferLength;
}

void Response::writeP(const unsigned char *data, size_t length) {
  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  while (length--) {
    write(pgm_read_byte(data++));
  }
}

void Response::m_init(Client *client) {
  m_stream = client;
  m_contentLenghtSet = false;
  m_contentTypeSet = false;
  m_keepAlive = false;
  m_statusSent = 0;
  m_headersSent = false;
  m_sendingStatus = false;
  m_sendingHeaders = false;
  m_bytesSent = 0;
  m_headersCount = 0;
  m_ended = false;
}

void Response::m_printStatus(int code) {
  switch (code) {
#ifndef LOW_SRAM_MCU
    case 100: {
      P(Continue) = "Continue";
      printP(Continue);
      break;
    }
    case 101: {
      P(SwitchingProtocols) = "Switching Protocols";
      printP(SwitchingProtocols);
      break;
    }
    case 102: {
      P(Processing) = "Processing";
      printP(Processing);
      break;
    }
    case 103: {
      P(EarlyHints) = "Early Hints";
      printP(EarlyHints);
      break;
    }
    case 200: {
      P(OK) = "OK";
      printP(OK);
      break;
    }
    case 201: {
      P(Created) = "Created";
      printP(Created);
      break;
    }
    case 202: {
      P(Accepted) = "Accepted";
      printP(Accepted);
      break;
    }
    case 203: {
      P(NonAuthoritativeInformation) = "Non-Authoritative Information";
      printP(NonAuthoritativeInformation);
      break;
    }
    case 204: {
      P(NoContent) = "No Content";
      printP(NoContent);
      break;
    }
    case 205: {
      P(ResetContent) = "Reset Content";
      printP(ResetContent);
      break;
    }
    case 206: {
      P(PartialContent) = "Partial Content";
      printP(PartialContent);
      break;
    }
    case 207: {
      P(MultiStatus) = "Multi-Status";
      printP(MultiStatus);
      break;
    }
    case 208: {
      P(AlreadyReported) = "Already Reported";
      printP(AlreadyReported);
      break;
    }
    case 226: {
      P(IMUsed) = "IM Used";
      printP(IMUsed);
      break;
    }
    case 300: {
      P(MultipleChoices) = "Multiple Choices";
      printP(MultipleChoices);
      break;
    }
    case 301: {
      P(MovedPermanently) = "Moved Permanently";
      printP(MovedPermanently);
      break;
    }
    case 302: {
      P(Found) = "Found";
      printP(Found);
      break;
    }
    case 303: {
      P(SeeOther) = "See Other";
      printP(SeeOther);
      break;
    }
    case 304: {
      P(NotModified) = "Not Modified";
      printP(NotModified);
      break;
    }
    case 305: {
      P(UseProxy) = "Use Proxy";
      printP(UseProxy);
      break;
    }
    case 306: {
      P(Unused) = "(Unused)";
      printP(Unused);
      break;
    }
    case 307: {
      P(TemporaryRedirect) = "Temporary Redirect";
      printP(TemporaryRedirect);
      break;
    }
    case 308: {
      P(PermanentRedirect) = "Permanent Redirect";
      printP(PermanentRedirect);
      break;
    }
    case 400: {
      P(BadRequest) = "Bad Request";
      printP(BadRequest);
      break;
    }
    case 401: {
      P(Unauthorized) = "Unauthorized";
      printP(Unauthorized);
      break;
    }
    case 402: {
      P(PaymentRequired) = "Payment Required";
      printP(PaymentRequired);
      break;
    }
    case 403: {
      P(Forbidden) = "Forbidden";
      printP(Forbidden);
      break;
    }
    case 404: {
      P(NotFound) = "Not Found";
      printP(NotFound);
      break;
    }
    case 405: {
      P(MethodNotAllowed) = "Method Not Allowed";
      printP(MethodNotAllowed);
      break;
    }
    case 406: {
      P(NotAcceptable) = "Not Acceptable";
      printP(NotAcceptable);
      break;
    }
    case 407: {
      P(ProxyAuthenticationRequired) = "Proxy Authentication Required";
      printP(ProxyAuthenticationRequired);
      break;
    }
    case 408: {
      P(RequestTimeout) = "Request Timeout";
      printP(RequestTimeout);
      break;
    }
    case 409: {
      P(Conflict) = "Conflict";
      printP(Conflict);
      break;
    }
    case 410: {
      P(Gone) = "Gone";
      printP(Gone);
      break;
    }
    case 411: {
      P(LengthRequired) = "Length Required";
      printP(LengthRequired);
      break;
    }
    case 412: {
      P(PreconditionFailed) = "Precondition Failed";
      printP(PreconditionFailed);
      break;
    }
    case 413: {
      P(PayloadTooLarge) = "Payload Too Large";
      printP(PayloadTooLarge);
      break;
    }
    case 414: {
      P(URITooLong) = "URI Too Long";
      printP(URITooLong);
      break;
    }
    case 415: {
      P(UnsupportedMediaType) = "Unsupported Media Type";
      printP(UnsupportedMediaType);
      break;
    }
    case 416: {
      P(RangeNotSatisfiable) = "Range Not Satisfiable";
      printP(RangeNotSatisfiable);
      break;
    }
    case 417: {
      P(ExpectationFailed) = "Expectation Failed";
      printP(ExpectationFailed);
      break;
    }
    case 421: {
      P(MisdirectedRequest) = "Misdirected Request";
      printP(MisdirectedRequest);
      break;
    }
    case 422: {
      P(UnprocessableEntity) = "Unprocessable Entity";
      printP(UnprocessableEntity);
      break;
    }
    case 423: {
      P(Locked) = "Locked";
      printP(Locked);
      break;
    }
    case 424: {
      P(FailedDependency) = "Failed Dependency";
      printP(FailedDependency);
      break;
    }
    case 425: {
      P(TooEarly) = "Too Early";
      printP(TooEarly);
      break;
    }
    case 426: {
      P(UpgradeRequired) = "Upgrade Required";
      printP(UpgradeRequired);
      break;
    }
    case 428: {
      P(PreconditionRequired) = "Precondition Required";
      printP(PreconditionRequired);
      break;
    }
    case 429: {
      P(TooManyRequests) = "Too Many Requests";
      printP(TooManyRequests);
      break;
    }
    case 431: {
      P(RequestHeaderFieldsTooLarge) = "Request Header Fields Too Large";
      printP(RequestHeaderFieldsTooLarge);
      break;
    }
    case 451: {
      P(UnavailableForLegalReasons) = "Unavailable For Legal Reasons";
      printP(UnavailableForLegalReasons);
      break;
    }
    case 500: {
      P(InternalServerError) = "Internal Server Error";
      printP(InternalServerError);
      break;
    }
    case 501: {
      P(NotImplemented) = "Not Implemented";
      printP(NotImplemented);
      break;
    }
    case 502: {
      P(BadGateway) = "Bad Gateway";
      printP(BadGateway);
      break;
    }
    case 503: {
      P(ServiceUnavailable) = "Service Unavailable";
      printP(ServiceUnavailable);
      break;
    }
    case 504: {
      P(GatewayTimeout) = "Gateway Timeout";
      printP(GatewayTimeout);
      break;
    }
    case 505: {
      P(HTTPVersionNotSupported) = "HTTP Version Not Supported";
      printP(HTTPVersionNotSupported);
      break;
    }
    case 506: {
      P(VariantAlsoNegotiates) = "Variant Also Negotiates";
      printP(VariantAlsoNegotiates);
      break;
    }
    case 507: {
      P(InsufficientStorage) = "Insufficient Storage";
      printP(InsufficientStorage);
      break;
    }
    case 508: {
      P(LoopDetected) = "Loop Detected";
      printP(LoopDetected);
      break;
    }
    case 510: {
      P(NotExtended) = "Not Extended";
      printP(NotExtended);
      break;
    }
    case 511: {
      P(NetworkAuthenticationRequired) = "Network Authentication Required";
      printP(NetworkAuthenticationRequired);
      break;
    }
#else
    case 200: {
      P(OK) = "OK";
      printP(OK);
      break;
    }
    case 201: {
      P(Created) = "Created";
      printP(Created);
      break;
    }
    case 202: {
      P(Accepted) = "Accepted";
      printP(Accepted);
      break;
    }
    case 204: {
      P(NoContent) = "No Content";
      printP(NoContent);
      break;
    }
    case 303: {
      P(SeeOther) = "See Other";
      printP(SeeOther);
      break;
    }
    case 304: {
      P(NotModified) = "Not Modified";
      printP(NotModified);
      break;
    }
    case 400: {
      P(BadRequest) = "Bad Request";
      printP(BadRequest);
      break;
    }
    case 401: {
      P(Unauthorized) = "Unauthorized";
      printP(Unauthorized);
      break;
    }
    case 402: {
      P(PaymentRequired) = "Payment Required";
      printP(PaymentRequired);
      break;
    }
    case 403: {
      P(Forbidden) = "Forbidden";
      printP(Forbidden);
      break;
    }
    case 404: {
      P(NotFound) = "Not Found";
      printP(NotFound);
      break;
    }
    case 405: {
      P(MethodNotAllowed) = "Method Not Allowed";
      printP(MethodNotAllowed);
      break;
    }
    case 406: {
      P(NotAcceptable) = "Not Acceptable";
      printP(NotAcceptable);
      break;
    }
    case 407: {
      P(ProxyAuthenticationRequired) = "Proxy Authentication Required";
      printP(ProxyAuthenticationRequired);
      break;
    }
    case 408: {
      P(RequestTimeout) = "Request Timeout";
      printP(RequestTimeout);
      break;
    }

    case 431: {
      P(RequestHeaderFieldsTooLarge) = "Request Header Fields Too Large";
      printP(RequestHeaderFieldsTooLarge);
      break;
    }
    case 500: {
      P(InternalServerError) = "Internal Server Error";
      printP(InternalServerError);
      break;
    }
    case 505: {
      P(HTTPVersionNotSupported) = "HTTP Version Not Supported";
      printP(HTTPVersionNotSupported);
      break;
    }
#endif
    default: {
      print(code);
      break;
    }
  }
}

bool Response::m_shouldPrintHeaders() {
  return (!m_headersSent && !m_sendingHeaders && !m_sendingStatus);
}

void Response::m_printHeaders() {
  setDefaults();
  beginHeaders();
  endHeaders();
}

void Response::m_printCRLF() { print(CRLF); }

void Response::m_flushBuf() {
  if (m_bufFill > 0) {
    if (m_headersSent && !m_contentLenghtSet) {
      m_stream->print(m_bufFill, HEX);
      m_stream->print(CRLF);
    }

    m_stream->write(m_buffer, m_bufFill);

    if (m_headersSent && !m_contentLenghtSet) {
      m_stream->print(CRLF);
    }

    m_bufFill = 0;
  };
}

void Response::m_reset() {
  m_flushBuf();

  if (m_headersSent && !m_contentLenghtSet) {
    m_stream->print(0, HEX);
    m_stream->print(CRLF);
    m_stream->print(CRLF);
  }
}

Request::Request()
    : m_stream(NULL),
      m_response(NULL),
      m_method(GET),
      m_minorVersion(-1),
      m_pushback(),
      m_pushbackDepth(0),
      m_readingContent(false),
      m_left(0),
      m_bytesRead(0),
      m_headerTail(NULL),
      m_query(NULL),
      m_queryLength(0),
      m_readTimedout(false),
      m_path(NULL),
      m_pathLength(0),
      m_pattern(NULL),
      m_route(NULL) {}

int Request::availableForWrite() {
  return m_response->availableForWrite();
}

int Request::available() {
  return min(m_stream->available(), m_left + m_pushbackDepth);
}

int Request::bytesRead() { return m_bytesRead; }

Stream *Request::stream() { return m_stream; }

char *Request::get(const char *name) {
  HeaderNode *headerNode = m_headerTail;

  while (headerNode != NULL) {
    if (Application::strcmpi(headerNode->name, name) == 0) {
      return headerNode->buffer;
    }

    headerNode = headerNode->next;
  }

  return NULL;
}

void Request::flush() { 
  return m_response->flush(); 
}

bool Request::form(char *name, int nameLength, char *value, int valueLength) {
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

char *Request::path() { return m_path; }

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
  if (m_pushbackDepth == SIZE(m_pushback)) {
    m_pushbackDepth = SIZE(m_pushback) - 1;
  }
}

char *Request::query() { return m_query; }

bool Request::query(const char *name, char *buffer, int bufferLength) {
  memset(buffer, 0, bufferLength);

  char *position = m_query;
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

  int read = m_stream->read(buf, (size < m_left ? size : m_left));
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

bool Request::route(const char *name, char *buffer, int bufferLength) {
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

bool Request::route(int number, char *buffer, int bufferLength) {
  memset(buffer, 0, bufferLength);
  int part = -1;
  const char *routeStart = m_route;

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

size_t Request::write(uint8_t data) {
  return m_response->write(data);
}

size_t Request::write(uint8_t* buffer, size_t bufferLength) {
  return m_response->write(buffer, bufferLength);
}

void Request::m_init(Client *client, Response *response, HeaderNode *headerTail, char *buffer,
                     int bufferLength, unsigned long timeout) {
  m_stream = client;
  m_response = response;
  m_bytesRead = 0;
  m_headerTail = headerTail;
  m_path = buffer;
  m_pathLength = bufferLength - 1;
  m_pushbackDepth = 0;
  m_left = 0;
  m_readTimedout = false;
  m_readingContent = false;
  m_method = GET;
  m_minorVersion = -1;

  _timeout = timeout;
}

bool Request::m_processMethod() {
  if (m_expect("GET ")) {
    m_method = GET;
  } else if (m_expect("HEAD ")) {
    m_method = HEAD;
  } else if (m_expect("POST ")) {
    m_method = POST;
  } else if (m_expect("PUT ")) {
    m_method = PUT;
  } else if (m_expect("DELETE ")) {
    m_method = DELETE;
  } else if (m_expect("PATCH ")) {
    m_method = PATCH;
  } else if (m_expect("OPTIONS ")) {
    m_method = OPTIONS;
  } else {
    return false;
  }

  return true;
}

bool Request::m_readURL() {
  char *request = m_path;
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
    if (m_expect("1.0")) {
      m_minorVersion = 0;
    } else if (m_expect("1.1")) {
      m_minorVersion = 1;
    } else if (m_timedRead() == -1) {
      return false;
    }
  }

  return true;
}

void Request::m_processURL() {
  char *qmLocation = strchr(m_path, '?');
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

    if (m_expect("Content-Length:")) {
      if (!m_readInt(m_left) || !m_expect(CRLF)) {
        return false;
      }

      canEnd = true;
    } else {
      HeaderNode *headerNode = m_headerTail;

      while (headerNode != NULL) {
        if (m_expect(headerNode->name) && m_expect(":")) {
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

bool Request::m_headerValue(char *buffer, int bufferLength) {
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

bool Request::m_readInt(int &number) {
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

void Request::m_setRoute(const char *route, const char *pattern) {
  m_route = route;
  m_pattern = pattern;
}

void Request::m_setMethod(MethodType method) { m_method = method; }

int Request::m_getUrlPathLength() { return m_pathLength; }

bool Request::m_expect(const char *expected) {
  const char *candidate = expected;
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

bool Request::m_skipSpace() {
  int ch;

  while ((ch = m_timedRead()) != -1 && (ch == ' ' || ch == '\t'))
    ;

  if (ch == -1) {
    return false;
  }

  push(ch);

  return true;
}

void Request::m_reset() {
  HeaderNode *headerNode = m_headerTail;
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

Router::Router()
    : m_head(NULL) {}

Router::~Router() {
  MiddlewareNode *current = m_head;
  MiddlewareNode *next;

  while (current != NULL) {
    next = current->next;
    delete current;

    current = next;
  }

  m_head = NULL;
}

void Router::del(const char *path, Middleware *middleware) {
  m_addMiddleware(Request::DELETE, path, middleware);
}

void Router::get(const char *path, Middleware *middleware) {
  m_addMiddleware(Request::GET, path, middleware);
}

void Router::head(const char *path, Middleware *middleware) {
  m_addMiddleware(Request::HEAD, path, middleware);
}

void Router::options(const char *path, Middleware *middleware) {
  m_addMiddleware(Request::OPTIONS, path, middleware);
}

void Router::post(const char *path, Middleware *middleware) {
  m_addMiddleware(Request::POST, path, middleware);
}

void Router::put(const char *path, Middleware *middleware) {
  m_addMiddleware(Request::PUT, path, middleware);
}

void Router::patch(const char *path, Middleware *middleware) {
  m_addMiddleware(Request::PATCH, path, middleware);
}

void Router::use(const char *path, Middleware *middleware) {
  m_addMiddleware(Request::USE, path, middleware);
}

void Router::use(Middleware *middleware) {
  use(NULL, middleware);
}

void Router::use(const char *path, Router *router) {
  MiddlewareNode *tail = new MiddlewareNode();
  tail->path = path;
  tail->middleware = NULL;
  tail->router = router;
  tail->next = NULL;
  m_mountMiddleware(tail);
}

void Router::use(Router *router) {
  use(NULL, router);
}

void Router::m_addMiddleware(Request::MethodType type, const char *path,
                             Middleware *middleware) {
  MiddlewareNode *tail = new MiddlewareNode();
  tail->path = path;
  tail->middleware = middleware;
  tail->router = NULL;
  tail->type = type;
  tail->next = NULL;

  m_mountMiddleware(tail);
}

void Router::m_mountMiddleware(MiddlewareNode *tail) {
  if (m_head == NULL) {
    m_head = tail;
  } else {
    MiddlewareNode *current = m_head;

    while (current->next != NULL) {
      current = current->next;
    }

    current->next = tail;
  }
}

void Router::m_dispatchMiddleware(Request &request, Response &response, int urlShift) {
  MiddlewareNode *middleware = m_head;

  while (middleware != NULL && !response.ended()) {
    if (middleware->router != NULL) {
      int prefixLength = middleware->path ? strlen(middleware->path) : 0;
      int shift = urlShift + prefixLength;

      if (middleware->path == NULL || strncmp(middleware->path, request.path() + urlShift, prefixLength) == 0) {
        middleware->router->m_dispatchMiddleware(request, response, shift);
      }
    } else if (middleware->type == request.method() || middleware->type == Request::USE) {
      if (middleware->path == NULL || m_routeMatch(request.path() + urlShift, middleware->path)) {
        request.m_setRoute(request.path() + urlShift, middleware->path);
        middleware->middleware(request, response);
      }
    }

    middleware = middleware->next;
  }
}

bool Router::m_routeMatch(const char* route, const char* pattern) {
  if (pattern[0] == '\0' && route[0] == '\0') {
    return true;
  }

  bool match = false;
  int i = 0;
  int j = 0;

  while (pattern[i] && route[j]) {
    if (pattern[i] == ':') {
      while (pattern[i] && pattern[i] != '/') {
        i++;
      }

      while (route[j] && route[j] != '/') {
        j++;
      }

      match = true;
    } else if (pattern[i] == route[j]) {
      j++;
      i++;
      match = true;
    } else {
      match = false;
      break;
    }
  }

  if (match && !pattern[i] && route[j] == '/' && !route[i]) {
    match = true;
  } else if (pattern[i] || route[j]) {
    match = false;
  }

  return match;
}

Application::Application()
    : m_headerTail(NULL), m_timeout(1000) {}

int Application::strcmpi(const char *s1, const char *s2) {
  int i;

  for (i = 0; s1[i] && s2[i]; ++i) {
    if (s1[i] == s2[i] || (s1[i] ^ 32) == s2[i]) {
      continue;
    } else {
      break;
    }
  }

  if (s1[i] == s2[i]) {
    return 0;
  }

  if ((s1[i] | 32) < (s2[i] | 32)) {
    return -1;
  }

  return 1;
}

Application::~Application() {
  Request::HeaderNode *current = m_headerTail;
  Request::HeaderNode *next;

  while (current != NULL) {
    next = current->next;
    delete current;
    current = next;
  }

  m_headerTail = NULL;
}

void Application::del(const char *path, Router::Middleware *middleware) {
  m_defaultRouter.m_addMiddleware(Request::DELETE, path, middleware);
}

void Application::get(const char *path, Router::Middleware *middleware) {
  m_defaultRouter.m_addMiddleware(Request::GET, path, middleware);
}

void Application::head(const char *path, Router::Middleware *middleware) {
  m_defaultRouter.m_addMiddleware(Request::HEAD, path, middleware);
}

void Application::options(const char *path, Router::Middleware *middleware) {
  m_defaultRouter.m_addMiddleware(Request::OPTIONS, path, middleware);
}

void Application::patch(const char *path, Router::Middleware *middleware) {
  m_defaultRouter.m_addMiddleware(Request::PATCH, path, middleware);
}

void Application::post(const char *path, Router::Middleware *middleware) {
  m_defaultRouter.m_addMiddleware(Request::POST, path, middleware);
}

void Application::put(const char *path, Router::Middleware *middleware) {
  m_defaultRouter.m_addMiddleware(Request::PUT, path, middleware);
}

void Application::process(Client *stream) {
  char request[SERVER_URL_BUFFER_SIZE];
  process(stream, request, SERVER_URL_BUFFER_SIZE);
}

void Application::process(Client *stream, char *buffer, int bufferLength) {
  if (stream == NULL) {
    return;
  }

  m_request.m_init(stream, &m_response, m_headerTail, buffer, bufferLength, m_timeout);
  m_response.m_init(stream);

  m_process();

  m_request.m_reset();
  m_response.m_reset();
}

void Application::process(Stream *stream) {
  StreamClient client(stream);
  process(&client);
}

void Application::process(Stream *stream, char *buffer, int bufferLength) {
  StreamClient client(stream);
  process(&client, buffer, bufferLength);
}

void Application::use(const char *path, Router::Middleware *middleware) {
  m_defaultRouter.m_addMiddleware(Request::USE, path, middleware);
}

void Application::use(Router::Middleware *middleware) {
  use(NULL, middleware);
}

void Application::setTimeout(unsigned long timeoutMillis) {
  m_timeout = timeoutMillis;
}

void Application::use(const char *path, Router *router) {
  m_defaultRouter.use(path, router);
}

void Application::use(Router *router) {
  use(NULL, router);
}

void Application::m_process() {
  if (!m_request.m_processMethod()) {
    if (m_request.m_timedout()) {
      return m_response.sendStatus(408);
    }

    return m_response.sendStatus(400);
  }

  if (!m_request.m_readURL()) {
    if (m_request.m_timedout()) {
      return m_response.sendStatus(408);
    }

    return m_response.sendStatus(414);
  }

  m_request.m_processURL();

  if (!m_request.m_readVersion()) {
    if (m_request.m_timedout()) {
      return m_response.sendStatus(408);
    }

    return m_response.sendStatus(505);
  }

  if (!m_request.m_processHeaders()) {
    if (m_request.m_timedout()) {
      return m_response.sendStatus(408);
    }

    return m_response.sendStatus(431);
  }

  m_defaultRouter.m_dispatchMiddleware(m_request, m_response);

  if (!m_response.statusSent() && !m_response.ended()) {
    return m_response.sendStatus(404);
  }

  if (!m_response.headersSent()) {
    m_response.m_printHeaders();
  }
}

void Application::header(const char *name, char *buffer, int bufferLength) {
  Request::HeaderNode *newNode = new Request::HeaderNode();

  buffer[0] = '\0';

  newNode->name = name;
  newNode->buffer = buffer;
  newNode->bufferLength = bufferLength;
  newNode->next = NULL;

  if (m_headerTail == NULL) {
    m_headerTail = newNode;
  } else {
    Request::HeaderNode *headerNode = m_headerTail;

    while (headerNode->next != NULL) {
      headerNode = headerNode->next;
    }

    headerNode->next = newNode;
  }
}
