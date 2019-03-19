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

Request::Request() :
  m_clientObject(NULL),
  m_method(GET),
  m_pushbackDepth(0),
  m_readingContent(false),
  m_left(0),
  m_bytesRead(0),
  m_headerTail(NULL),
  m_query(NULL),
  m_queryLength(0),
  m_timeout(false),
  m_path(NULL),
  m_pathLength(0),
  m_route(NULL) {
}

int Request::available() {
  return m_clientObject->available();
}

bool Request::body(uint8_t *buffer, int bufferLength) {
  memset(buffer, 0, bufferLength);

  while (left() && --bufferLength && !m_timeout) {
    *buffer++ = read();
  }

  return !left();
}

int Request::bytesRead() {
  return m_bytesRead;
}

Client *Request::client() {
  return m_clientObject;
}

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

bool Request::form(char *name, int nameLength, char *value, int valueLength) {
  int ch;
  bool foundSomething = false;
  bool readingName = true;

  memset(name, 0, nameLength);
  memset(value, 0, valueLength);

  while ((ch = read()) != -1) {
    foundSomething = true;
    if (ch == '+') {
      ch = ' ';
    } else if (ch == '=') {
      readingName = false;
      continue;
    } else if (ch == '&') {
      return nameLength > 0 && valueLength > 0;
    } else if (ch == '%') {
      char high = read();
      if (high == -1) {
        return false;
      }

      char low = read();
      if (high == -1) {
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

int Request::left() {
  return m_left;
}

Request::MethodType Request::method() {
  return m_method;
}

char *Request::path() {
  return m_path;
}

int Request::peek() {
  uint8_t c = read();
  push(c);

  return c;
}

void Request::push(uint8_t ch) {
  if (ch == -1) {
    return;
  }

  m_pushback[m_pushbackDepth++] = ch;

  // can't raise error here, so just replace last char over and over
  if (m_pushbackDepth == SIZE(m_pushback)) {
    m_pushbackDepth = SIZE(m_pushback) - 1;
  }
}

char *Request::query() {
  return m_query;
}

bool Request::query(const char *name, char *buffer, int bufferLength) {
  memset(buffer, 0, bufferLength);

  char * position = m_query;
  int nameLength = strlen(name);

  while ((position = strstr(position, name))) {
    char previous = *(position - 1);

    if ((previous == '\0' || previous == '&') && *(position + nameLength) == '=') {
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
  if (m_timeout) {
    return -1;
  }
  m_timeout = false;

  if (m_pushbackDepth > 0) {
    return m_pushback[--m_pushbackDepth];
  }

  unsigned long timeoutTime = millis() + SERVER_READ_TIMEOUT_MS;

  while (m_clientObject->connected()) {
    if (m_readingContent && left() == 0) {
      return -1;
    }

    int ch = -1;
    if (m_clientObject->available()) {
      ch = m_clientObject->read();
    }

    if (ch != -1) {
      m_bytesRead++;

      if (m_readingContent) {
        --m_left;
      }

      return ch;
    } else {
      unsigned long now = millis();

      if (now > timeoutTime) {
        m_timeout = true;
        return -1;
      }
    }
  }

  m_timeout = true;
  return -1;
}

bool Request::route(const char *name, char *buffer, int bufferLength) {
  int part = 0;
  int i = 1;

  while (m_route[i]) {
    if (m_route[i] == '/') {
      part++;
    }

    if (m_route[i++] == ':') {
      int j = 0;

      while ((m_route[i] && name[j]) && m_route[i] == name[j]) {
        i++;
        j++;
      }

      if (!name[j] && (m_route[i] == '/' || !m_route[i])) {
        return route(part, buffer, bufferLength);
      }
    }
  }

  return false;
}

bool Request::route(int number, char *buffer, int bufferLength) {
  memset(buffer, 0, bufferLength);
  int part = -1;
  char *routeStart = m_path + m_prefixLength;

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

bool Request::timeout() {
  return m_timeout;
}

size_t Request::write(uint8_t data) {
  return 0;
}

void Request::flush() {
  return;
}

void Request::m_init(Client *client, char *buffer, int bufferLength) {
  m_clientObject = client;
  m_bytesRead = 0;
  m_path = buffer;
  m_pathLength = bufferLength - 1;
  m_pushbackDepth = 0;
  m_left = 0;
  m_timeout = false;
  m_readingContent = false;
  m_method = GET;
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

  while ((ch = read()) != -1 && ch != ' ' && ch != '\n' && ch != '\r' && --bufferLeft) {
    if (ch == '%') {
      char high = read();
      if (high == -1) {
        return false;
      }

      char low = read();
      if (high == -1) {
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

bool Request::m_processHeaders(HeaderNode *headerTail) {
  m_headerTail = headerTail;

  while (true) {
    if (m_expect("Content-Length:")) {
      m_readInt(m_left);
      continue;
    }

    bool match = false;
    HeaderNode *headerNode = m_headerTail;

    while (headerNode != NULL) {
      if (m_expect(headerNode->name)) {
        char c = read();
        if (c  == -1) {
          return false;
        }

        if (c == ':') {
          if (!m_headerValue(headerNode->buffer, headerNode->bufferLength)) {
            return false;
          }

          match = true;
          break;
        } else {
          push(c);
        }
      }

      headerNode = headerNode->next;
    }

    if (match) {
      continue;
    }

    if (m_expect(CRLF CRLF)) {
      m_readingContent = true;
      return true;
    }

    if (read() == -1) {
      return false;
    }
  }
}

bool Request::m_headerValue(char *buffer, int bufferLength) {
  int ch;
  memset(buffer, 0, bufferLength);

  while ((ch = read()) != -1 && (ch == ' ' || ch == '\t'));
  push(ch);

  while ((ch = read()) != -1 && ch != '\r') {
    if (--bufferLength > 0) {
      *buffer++ = ch;
    }
  }

  return bufferLength > 0;
}

bool Request::m_readInt(int &number) {
  bool negate = false;
  bool gotNumber = false;
  int ch;

  number = 0;

  while ((ch = read()) != -1 && (ch == ' ' || ch == '\t'));

  if (ch == '-') {
    negate = true;
    ch = read();
    if (m_timeout) {
      return false;
    }
  }

  while (ch >= '0' && ch <= '9') {
    gotNumber = true;
    number = number * 10 + ch - '0';
    ch = read();
    if (m_timeout) {
      return false;
    }
  }

  push(ch);

  if (negate) {
    number = -number;
  }

  return gotNumber;
}

void Request::m_setRoute(int prefixLength, const char *route) {
  m_prefixLength = prefixLength;
  m_route = route;
}

void Request::m_setMethod(MethodType method) {
  m_method = method;
}

int Request::m_getUrlPathLength() {
  return m_pathLength;
}

bool Request::m_expect(const char *expected) {
  const char *candidate = expected;
  while (*candidate != 0) {
    int ch = read();

    if (ch != *candidate++) {
      push(ch);

      while (--candidate != expected) {
        push(candidate[-1]);
      }

      return false;
    }
  }

  return true;
}

void Request::m_reset() {
  HeaderNode *headerNode = m_headerTail;
  while (headerNode != NULL) {
    headerNode->buffer[0] = '\0';
    headerNode = headerNode->next;
  }
}

Response::Response():
  m_clientObject(NULL),
  m_contentTypeSet(false),
  m_statusSent(false),
  m_headersSent(false),
  m_sendingStatus(false),
  m_sendingHeaders(false),
  m_headersCount(0),
  m_bytesSent(0),
  m_ended(false),
  m_bufFill(0) {
}

int Response::bytesSent() {
  return m_bytesSent;
}

void Response::end() {
  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }
  m_ended = true;
}

bool Response::ended() {
  return m_ended;
}

void Response::flush() {
  m_flushBuf();
  m_clientObject->flush();
}

const char * Response::get(const char *name) {
  for (int i = 0; i < m_headersCount; i++) {
    if (Application::strcmpi(name, m_headers[i].name) == 0) {
      return m_headers[m_headersCount].value;
    }
  }

  return NULL;
}

bool Response::headersSent() {
  return m_headersSent;
}

void Response::printP(const unsigned char *string) {
  if (m_ended) {
    return;
  }

  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  while (uint8_t value = pgm_read_byte(string++)) {
    write(value);
  }
}

void Response::printP(const char *string) {
  printP((unsigned char*) string);
}

void Response::sendStatus(int code) {
  status(code);
  m_printHeaders();
  m_printStatus(code);
}

void Response::set(const char *name, const char *value) {
  for (int i = 0; i < m_headersCount; i++) {
    if (Application::strcmpi(name, m_headers[i].name) == 0) {
      m_headers[m_headersCount].value = value;
      return;
    }
  }

  if (m_headersCount >= SIZE(m_headers)) {
    return;
  }

  m_headers[m_headersCount].name = name;
  m_headers[m_headersCount].value = value;
  m_headersCount++;

  if (Application::strcmpi(name, "Content-Type") == 0) {
    m_contentTypeSet = true;
  }
}

void Response::status(int code) {
  if (m_statusSent) {
    return;
  }

  m_sendingStatus = true;
  P(httpVersion) = "HTTP/1.0 ";
  printP(httpVersion);
  print(code);
  print(" ");
  m_printStatus(code);

  m_printCRLF();
  m_statusSent = true;
  m_sendingStatus = false;
}

bool Response::statusSent() {
  return m_statusSent;
}

size_t Response::write(uint8_t data) {
  if (m_ended) {
    return 0;
  }

  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  m_buffer[m_bufFill++] = data;

  if (m_bufFill == SERVER_OUTPUT_BUFFER_SIZE) {
    m_clientObject->write(m_buffer, SERVER_OUTPUT_BUFFER_SIZE);
    m_bufFill = 0;
  }

  size_t bytesSent = sizeof(data);
  m_bytesSent += bytesSent;
  return bytesSent;
}

size_t Response::write(uint8_t *buffer, size_t bufferLength) {
  if (m_ended) {
    return 0;
  }

  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  m_flushBuf();
  m_clientObject->write(buffer, bufferLength);
  m_bytesSent += bufferLength;
  return bufferLength;
}

void Response::writeP(const unsigned char *data, size_t length) {
  if (m_ended) {
    return;
  }

  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  while (length--) {
    write(pgm_read_byte(data++));
  }
}

void Response::m_init(Client * client) {
  m_clientObject = client;
  m_contentTypeSet = false;
  m_statusSent = false;
  m_headersSent = false;
  m_sendingStatus = false;
  m_sendingHeaders = false;
  m_bytesSent = 0;
  m_headersCount = 0;
  m_ended = false;
}

int Response::available() {
  return 0;
}

int Response::read() {
  return -1;
}

int Response::peek() {
  return -1;
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
  m_sendingHeaders = true;

  if (!m_statusSent) {
    status(200);
  }

  if (!m_contentTypeSet) {
    set("Content-Type", "text/plain");
  }

  for (int i = 0; i < m_headersCount; i++) {
    print(m_headers[i].name);
    print(": ");
    print(m_headers[i].value);
    m_printCRLF();
  }

  m_printCRLF();
  m_sendingHeaders = false;
  m_headersSent = true;
}

void Response::m_printCRLF() {
  print(CRLF);
}

void Response::m_flushBuf() {
  if (m_bufFill > 0) {
    m_clientObject->write(m_buffer, m_bufFill);
    m_bufFill = 0;
  };
}

void Response::m_reset() {
  flush();
  m_clientObject->stop();
}


Router::Router(const char *urlPrefix):
  m_head(NULL),
  m_next(NULL),
  m_urlPrefix(urlPrefix) {
}

void Router::all(const char *path, Middleware * middleware) {
  m_addMiddleware(Request::ALL, path, middleware);
}

void Router::del(const char *path, Middleware * middleware) {
  m_addMiddleware(Request::DELETE, path, middleware);
}

void Router::get(const char *path, Middleware * middleware) {
  m_addMiddleware(Request::GET, path, middleware);
}

void Router::options(const char *path, Middleware * middleware) {
  m_addMiddleware(Request::OPTIONS, path, middleware);
}

void Router::post(const char *path, Middleware * middleware) {
  m_addMiddleware(Request::POST, path, middleware);
}

void Router::put(const char *path, Middleware * middleware) {
  m_addMiddleware(Request::PUT, path, middleware);
}

void Router::patch(const char *path, Middleware * middleware) {
  m_addMiddleware(Request::PATCH, path, middleware);
}

void Router::use(Middleware * middleware) {
  m_addMiddleware(Request::USE, NULL, middleware);
}

void Router::m_addMiddleware(Request::MethodType type, const char *path, Middleware * middleware) {
  MiddlewareNode *tail = (MiddlewareNode *)malloc(sizeof(MiddlewareNode));
  tail->path = path;
  tail->middleware = middleware;
  tail->type = type;
  tail->next = NULL;

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

Router *Router::m_getNext() {
  return m_next;
}

void Router::m_setNext(Router * next) {
  m_next = next;
}

bool Router::m_dispatchMiddleware(Request & request, Response & response) {
  bool routeFound = false;
  int prefixLength = strlen(m_urlPrefix);

  if (strncmp(m_urlPrefix, request.path(), prefixLength) == 0) {
    char *trimmedPath = request.path() + prefixLength;

    MiddlewareNode *middleware = m_head;

    while (middleware != NULL && !response.ended()) {
      if (middleware->type == request.method() || middleware->type == Request::ALL ||
          middleware->type == Request::USE) {
        if (middleware->type == Request::USE ||
            m_routeMatch(trimmedPath, middleware->path)) {
          if (middleware->type != Request::USE) {
            routeFound = true;
          }

          request.m_setRoute(prefixLength, middleware->path);
          middleware->middleware(request, response);
        }
      }

      middleware = middleware->next;
    }
  }

  return routeFound;
}

bool Router::m_routeMatch(const char *text, const char *pattern) {
  if (pattern[0] == '\0' && text[0] == '\0') {
    return true;
  }

  boolean match = false;
  int i = 0;
  int j = 0;

  while (pattern[i] && text[j]) {
    if (pattern[i] == ':') {
      while (pattern[i] && pattern[i] != '/') {
        i++;
      }

      while (text[j] && text[j] != '/') {
        j++;
      }

      match = true;
    } else if (pattern[i] == text[j]) {
      j++;
      i++;
      match = true;
    } else {
      match = false;
      break;
    }
  }

  if (match && !pattern[i] && text[j] == '/' && !text[j]) {
    match = true;
  } else if (pattern[i] || text[j]) {
    match = false;
  }

  return match;
}


Application::Application():
  m_clientObject(NULL),
  m_routerTail(&m_defaultRouter),
  m_headerTail(NULL) {
}

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

void Application::all(const char *path, Router::Middleware * middleware) {
  m_defaultRouter.m_addMiddleware(Request::ALL, path, middleware);
}

void Application::del(const char *path, Router::Middleware * middleware) {
  m_defaultRouter.m_addMiddleware(Request::DELETE, path, middleware);
}

void Application::get(const char *path, Router::Middleware * middleware) {
  m_defaultRouter.m_addMiddleware(Request::GET, path, middleware);
}

void Application::options(const char *path, Router::Middleware * middleware) {
  m_defaultRouter.m_addMiddleware(Request::OPTIONS, path, middleware);
}

void Application::patch(const char *path, Router::Middleware * middleware) {
  m_defaultRouter.m_addMiddleware(Request::PATCH, path, middleware);
}

void Application::post(const char *path, Router::Middleware * middleware) {
  m_defaultRouter.m_addMiddleware(Request::POST, path, middleware);
}

void Application::put(const char *path, Router::Middleware * middleware) {
  m_defaultRouter.m_addMiddleware(Request::PUT, path, middleware);
}

void Application::process(Client * client) {
  char request[SERVER_URL_BUFFER_SIZE];
  process(client, request, SERVER_URL_BUFFER_SIZE);
}

void Application::process(Client * client, char *buffer, int bufferLength) {
  m_clientObject = client;

  if (m_clientObject == NULL) {
    return;
  }

  m_request.m_init(m_clientObject, buffer, bufferLength);
  m_response.m_init(m_clientObject);

  m_process();

  m_request.m_reset();
  m_response.m_reset();
}

void Application::use(Router::Middleware * middleware) {
  m_defaultRouter.m_addMiddleware(Request::USE, NULL, middleware);
}

/* Mounts a Router instance to the server. */
void Application::route(Router * router) {
  Router *routerNode = m_routerTail;

  while (routerNode->m_getNext() != NULL) {
    routerNode = routerNode->m_getNext();
  }

  routerNode->m_setNext(router);
}

void Application::m_process() {
  bool routeMatch = false;

  if (!m_request.m_processMethod()) {
    if (m_request.timeout()) {
      return m_response.sendStatus(408);
    }

    return m_response.sendStatus(400);
  }

  if (!m_request.m_readURL()) {
    if (m_request.timeout()) {
      return m_response.sendStatus(408);
    }

    return m_response.sendStatus(414);
  }

  m_request.m_processURL();

  if (!m_request.m_processHeaders(m_headerTail)) {
    if (m_request.timeout()) {
      return m_response.sendStatus(408);
    }

    return m_response.sendStatus(431);
  }

  if (m_request.method() == Request::HEAD) {
    m_request.m_setMethod(Request::GET);
    m_response.end();
  }

  Router *routerNode = m_routerTail;

  while (routerNode != NULL && !m_response.ended()) {
    if (routerNode->m_dispatchMiddleware(m_request, m_response)) {
      routeMatch = true;
    }

    routerNode = routerNode->m_getNext();
  }

  if (!routeMatch && !m_response.ended()) {
    return m_response.sendStatus(404);
  }

  if (!m_response.ended() && !m_response.headersSent()) {
    m_response.m_printHeaders();
  }
}

void Application::header(const char *name, char *buffer, int bufferLength) {
  Request::HeaderNode *newNode =
    (Request::HeaderNode *)malloc(sizeof(Request::HeaderNode));

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