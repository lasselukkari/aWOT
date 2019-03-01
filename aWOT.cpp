/*
  Web server library for Arduino, Teensy and ESP8266
  Copyright 2014 Lasse Lukkari

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

/* Request constructor. */
Request::Request() :
  m_clientObject(NULL),
  m_methodType(INVALID),
  m_pushbackDepth(0),
  m_readingContent(false),
  m_contentLeft(0),
  m_bytesRead(0),
  m_headerTail(NULL),
  m_query(NULL),
  m_queryComplete(false),
  m_urlPath(NULL),
  m_urlPathLength(0),
  m_route(NULL) {
}

/* Initializes the request instance ready to process the incoming HTTP request. */
void Request::m_init(Client *client, char* buff, int bufflen) {
  m_clientObject = client;
  m_bytesRead = 0;
  m_urlPath = buff;
  m_urlPathLength = bufflen - 1;
  m_pushbackDepth = 0;
  m_contentLeft = 0;
  m_readingContent = false;
  m_methodType = INVALID;
}

/* Processes the first line of the HTTP request to parse method, verb and query. */
void Request::m_processRequestLine() {
  char * request = m_urlPath;
  int bufferLeft = m_urlPathLength;

  // store the HTTP method line of the request
  if (m_expect("GET ")) {
    m_methodType = GET;
  } else if (m_expect("HEAD ")) {
    m_methodType = HEAD;
  } else if (m_expect("POST ")) {
    m_methodType = POST;
  } else if (m_expect("PUT ")) {
    m_methodType = PUT;
  } else if (m_expect("DELETE ")) {
    m_methodType = DELETE;
  } else if (m_expect("PATCH ")) {
    m_methodType = PATCH;
  } else if (m_expect("OPTIONS ")) {
    m_methodType = OPTIONS;
  } else {
    return;
  }

  int ch;
  while ((ch = read()) != -1) {
    if (ch == ' ' || ch == '\n' || ch == '\r') {
      break;
    }

    if (bufferLeft-- > 0) {
      *request = ch;
      request++;
    }
  }
  *request = 0;

  m_queryComplete = bufferLeft;
}

void Request::m_processURL() {
  char * qmLocation = strchr(m_urlPath, '?');
  int qmOffset = (qmLocation == NULL) ? 0 : 1;

  m_urlPathLength =
    (qmLocation == NULL) ? strlen(m_urlPath) : (qmLocation - m_urlPath);
  m_query = m_urlPath + m_urlPathLength + qmOffset;

  if (qmOffset) {
    *qmLocation = 0;
  }
}

void Request::m_decodeURL() {
  char *leader = m_urlPath;
  char *follower = leader;

  while (*leader) {
    if (*leader == '%') {
      leader++;
      char high = *leader;
      leader++;
      char low = *leader;

      if (high > 0x39) {
        high -= 7;
      }

      high &= 0x0f;

      if (low > 0x39) {
        low -= 7;
      }

      low &= 0x0f;

      *follower = (high << 4) | low;
    } else {
      *follower = *leader;
    }

    leader++;
    follower++;
  }

  *follower = 0;
}

/* Processes the header fields of the request */
void Request::m_processHeaders(HeaderNode* headerTail) {
  m_headerTail = headerTail;

  while (true) {
    if (m_expect("Content-Length:")) {
      m_readInt(m_contentLeft);
      continue;
    }

    bool match = false;
    HeaderNode* headerNode = m_headerTail;

    while (headerNode != NULL) {
      if (m_expect(headerNode->name) ) {
        char c = read();

        if (c == ':') {
          m_readHeader(headerNode->buffer, headerNode->size);
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
      return;
    }

    // no expect checks hit, so just absorb a character and try again
    if (read() == -1) {
      return;
    }
  }
}

/*Returns the method type of the current HTTP request.*/
Request::MethodType Request::method() {
  return m_methodType;
}

/*Returns the number of bytes available for reading.*/
int Request::contentLeft() {
  return m_contentLeft;
}

char * Request::urlPath() {
  return m_urlPath;
}

int Request::m_getUrlPathLength() {
  return m_urlPathLength;
}

bool Request::route(const char * name, char *paramBuffer, int paramBufferLen) {
  int part = 0;
  int i = 1;

  while (m_route[i]) {
    if (m_route[i] == '/') {
      part++;
    }

    if (m_route[i++] == ':') {
      int j = 0;

      while ((m_route[i] && name[j]) && m_route[i++] == name[j++])

        if (!name[j] && (m_route[i] == '/' || !m_route[i])) {
          return route(part, paramBuffer, paramBufferLen);
        }
    }
  }

  return false;
}

bool Request::route(int number, char *paramBuffer, int paramBufferLen) {
  memset(paramBuffer, 0, paramBufferLen);
  int part = 0;
  int i = 1;
  int read = 0;
  char * routeStart = m_urlPath + m_prefixLength;

  while (routeStart[i]) {
    if (routeStart[i++] == '/') {
      part++;

      if (part == number) {
        while (routeStart[i] != '/' && read < paramBufferLen) {
          paramBuffer[read++] = routeStart[i++];
        }

        return true;
      }
    }
  }

  return false;
}

/* Return a char pointer to the request query placed after the ? character in the URL */
char * Request::query() {
  return m_query;
}

/* Returns a single query parameter by name. For example with  request to URL /search?query=word request.query("query")
   would return a char pointer to "word" */
bool Request::query(const char * key, char *paramBuffer, int paramBufferLen) {
  memset(paramBuffer, 0, paramBufferLen);
  int charsRead = 0;

  char *ch = strstr(m_query, key);

  if (ch) {
    ch += strlen(key);

    if (*ch == '=') {
      ch++;

      while (*ch && *ch != '&' && charsRead < paramBufferLen) {
        paramBuffer[charsRead++] = *ch++;
      }

      return true;
    }
  }

  return false;
}

/* Returns a boolean value indicating if the query was parsed completely or didn't it fit to the  buffer */
bool Request::queryComplete() {
  return m_queryComplete;
}

/* Reads the next available POST parameter name and value from the request body */
bool Request::postParam(char *name, int nameLen, char *value, int valueLen) {
  int ch;
  bool foundSomething = false;
  memset(name, 0, nameLen);
  memset(value, 0, valueLen);
  --nameLen;
  --valueLen;

  while ((ch = read()) != -1) {
    foundSomething = true;

    if (ch == '+') {
      ch = ' ';
    }

    else if (ch == '=') {
      nameLen = 0;
      continue;
    }

    else if (ch == '&') {
      return true;
    }

    else if (ch == '%') {

      char ch1 = read();
      char ch2 = read();

      if (ch1 == -1 || ch2 == -1) {
        return false;
      }

      char hex[3] = { ch1, ch2, 0 };
      ch = m_hexToInt(hex);

    }

    if (nameLen > 0) {
      *name++ = ch;
      --nameLen;
    }

    else if (valueLen > 0) {
      *value++ = ch;
      --valueLen;
    }
  }

  if (foundSomething) {
    return true;
  } else {
    return false;
  }
}

/* Returns a pointer to a header value */
char * Request::header(const char *name) {
  HeaderNode* headerNode = m_headerTail;

  while (headerNode != NULL) {
    if (strcmp(headerNode->name, name) == 0) {
      return headerNode->buffer;
    }

    headerNode = headerNode->next;
  }

  return NULL;
}

void Request::m_setRoute(int prefixLength, const char * routeString) {
  m_prefixLength = prefixLength;
  m_route = routeString;
}

/*Returns the number of bytes available for reading.*/
int Request::available() {
  return m_clientObject->available();
}

int Request::read() {
  if (m_clientObject == NULL) {
    return -1;
  }

  if (m_pushbackDepth == 0) {
    unsigned long timeoutTime = millis() + SERVER_READ_TIMEOUT_IN_MS;

    while (m_clientObject->connected()) {
      if (m_readingContent) {
        if (m_contentLeft == 0) {
          return -1;
        }
      }

      int ch = m_clientObject->read();

      if (ch != -1) {
        m_bytesRead++;

        if (m_readingContent) {
          --m_contentLeft;
        }

        return ch;
      }

      else {
        unsigned long now = millis();

        if (now > timeoutTime) {
          m_reset();
          return -1;
        }
      }
    }

    return -1;
  } else {
    return m_pushback[--m_pushbackDepth];
  }
}

int Request::bytesRead() {
  return m_bytesRead;
}

int Request::peek() {
  int c = read();
  push(c);

  return c;
}

void Request::push(int ch) {
  if (ch == -1) {
    return;
  }

  m_pushback[m_pushbackDepth++] = ch;

  if (m_pushbackDepth == SIZE(m_pushback)) {
    m_pushbackDepth = SIZE(m_pushback) - 1;
  }
}

bool Request::m_expect(const char *str) {
  const char *curr = str;

  while (*curr != 0) {
    int ch = read();

    if (tolower(ch) != tolower(*curr++)) {
      push(ch);

      while (--curr != str) {
        push(curr[-1]);
      }

      return false;
    }
  }

  return true;
}

void Request::m_reset() {
  HeaderNode* headerNode = m_headerTail;
  while (headerNode != NULL) {
    headerNode->buffer[0] = '\0';
    headerNode = headerNode->next;
  }
}

void Request::m_readHeader(char *value, int valueLen) {
  int ch;
  memset(value, 0, valueLen);
  --valueLen;

  do {
    ch = read();
  } while (ch == ' ' || ch == '\t');

  do {
    if (valueLen > 0) {
      *value++ = ch;
      --valueLen;
    }

    ch = read();
  } while (ch != '\r');

  push(ch);
}

bool Request::m_readInt(int &number) {
  bool negate = false;
  bool gotNumber = false;
  int ch;

  number = 0;

  // absorb whitespace
  do {
    ch = read();
  } while (ch == ' ' || ch == '\t');

  // check for leading minus sign
  if (ch == '-') {
    negate = true;
    ch = read();
  }

  // read digits to update number, exit when we find non-digit
  while (ch >= '0' && ch <= '9') {
    gotNumber = true;
    number = number * 10 + ch - '0';
    ch = read();
  }

  push(ch);

  if (negate) {
    number = -number;
  }

  return gotNumber;
}

int Request::m_hexToInt(char *hex) {
  int converted = 0;

  while (true) {
    char c = *hex;

    if (c >= '0' && c <= '9') {
      converted *= 16;
      converted += c - '0';
    } else if (c >= 'a' && c <= 'f') {
      converted *= 16;
      converted += (c - 'a') + 10;
    } else if (c >= 'A' && c <= 'F') {
      converted *= 16;
      converted += (c - 'A') + 10;
    }  else {
      break;
    }

    hex++;
  }

  return converted;
}

/* Request class constructor. */
Response::Response() :
  m_clientObject(NULL),
  m_headersCount(0),
  m_bytesSent(0),
  m_ended(false),
  m_bufFill(0) {
}

/* Initializes the request instance ready for outputting the HTTP response. */
void Response::m_init(Client *client) {
  m_clientObject = client;
  m_bytesSent = 0;
  m_headersCount = 0;
  m_ended = false;
}

void Response::writeP(const unsigned char *data, size_t length) {
  while (length--) {
    write(pgm_read_byte(data++));
  }
}

void Response::printP(const unsigned char *str) {
  while (uint8_t value = pgm_read_byte(str++)) {
    write(value);
  }
}

size_t Response::write(uint8_t ch) {
  m_buffer[m_bufFill++] = ch;

  if (m_bufFill == SERVER_OUTPUT_BUFFER_SIZE) {
    m_clientObject->write(m_buffer, SERVER_OUTPUT_BUFFER_SIZE);
    m_bufFill = 0;
  }

  size_t bytesSent = sizeof(ch);
  m_bytesSent += bytesSent;
  return bytesSent;
}

size_t Response::write(uint8_t* buffer, size_t size) {
  m_flushBuf();
  m_clientObject->write(buffer, size);
  m_bytesSent += size;
  return size;
}

void Response::flush() {
  m_flushBuf();
  m_clientObject->flush();
}

int Response::bytesSent() {
  return m_bytesSent;
}

void Response::end() {
  m_ended = true;
}

bool Response::ended() {
  return m_ended;
}

/* Sets a header name and value pair to the response. */
void Response::set(const char *name, const char *value) {
  if (m_headersCount < SIZE(m_headers)) {
    m_headers[m_headersCount].name = name;
    m_headers[m_headersCount].value = value;
    m_headersCount++;
  }
}

/* Sends default status and headers indicating a successful request. */
void Response::success(const char *contentType) {
  P(successMsg1) =
    "HTTP/1.0 200 OK" CRLF
    "Content-Type: ";

  printP(successMsg1);
  print(contentType);
  m_printCRLF();
  m_printHeaders();
  m_printCRLF();
}

/* Sends default status and headers indicating a creation of an resource. */
void Response::created(const char *contentType) {
  P(successMsg1) =
    "HTTP/1.0 201 Created" CRLF
    "Content-Type: ";

  printP (successMsg1);
  print(contentType);
  m_printCRLF();
  m_printHeaders();
  m_printCRLF();
}

/* Sends default status and headers indicating a successful request without any response body. */
void Response::noContent() {
  P(noContentMsg) = "HTTP/1.0 204 No Content" CRLF;

  printP(noContentMsg);
  m_printHeaders();
  m_printCRLF();
}

/* Sends redirection response. */
void Response::seeOther(const char *otherURL) {
  P(seeOtherMsg) =
    "HTTP/1.0 303 See Other" CRLF
    "Location: ";

  printP (seeOtherMsg);
  print(otherURL);
  m_printHeaders();
  m_printCRLF();
}

void Response::notModified() {
  P(noContentMsg) = "HTTP/1.0 304 Not Modified" CRLF;

  printP(noContentMsg);
  m_printHeaders();
  m_printCRLF();
}

/* Sends a default error response for a failed request. */
void Response::fail() {
  P(failMsg1) = "HTTP/1.0 400 Bad Request" CRLF;
  P(failMsg2) = "Content-Type: text/html" CRLF
                CRLF SERVER_FAIL_MESSAGE;

  printP(failMsg1);
  m_printHeaders();
  printP(failMsg2);
}

/* Sends a default error response for a unauthorized request. */
void Response::unauthorized() {
  P(failMsg1) =
    "HTTP/1.0 401 Unauthorized" CRLF
    "Content-Type: text/html" CRLF;
  P(failMsg2) = CRLF SERVER_AUTH_MESSAGE;

  printP(failMsg1);
  m_printHeaders();
  printP(failMsg2);
}

/* Sends a default error response for a unauthorized request. */
void Response::forbidden() {
  P(failMsg1) =
    "HTTP/1.0 403 Forbidden" CRLF
    "Content-Type: text/html" CRLF;
  P(failMsg2) = CRLF SERVER_FORBIDDEN_MESSAGE;

  printP(failMsg1);
  m_printHeaders();
  printP(failMsg2);
}

/* Sends a default error response for request targeted to an nonexistent resource. */
void Response::notFound() {
  P(failMsg1) =
    "HTTP/1.0 404 Not Found" CRLF
    "Content-Type: text/html" CRLF;
  P(failMsg2) = CRLF SERVER_NOT_FOUND_MESSAGE;

  printP(failMsg1);
  m_printHeaders();
  printP(failMsg2);
}

/* Sends a default default error response for a failed request handling. */
void Response::serverError() {
  P(failMsg1) =
    "HTTP/1.0 500 Internal Server Error" CRLF
    "Content-Type: text/html" CRLF;
  P(failMsg2) = CRLF SERVER_SERVER_ERROR_MESSAGE;

  printP(failMsg1);
  m_printHeaders();
  printP(failMsg2);
}

void Response::m_reset() {
  flush();
  m_clientObject->stop();
}

void Response::m_printCRLF() {
  print(CRLF);
}

void Response::m_printHeaders() {
  for (byte i = 0; i < m_headersCount; i++) {
    print(m_headers[i].name);
    print(": ");
    print(m_headers[i].value);
    m_printCRLF();
  }
}

void Response::m_flushBuf() {
  if (m_bufFill > 0) {
    m_clientObject->write(m_buffer, m_bufFill);
    m_bufFill = 0;
  };
}

/* Router class constructor with an optional URL prefix parameter */
Router::Router(const char * urlPrefix) :
  m_tailCommand(NULL),
  m_next(NULL),
  m_urlPrefix(urlPrefix) {
}

bool Router::m_dispatchCommands(Request& request, Response& response) {
  bool routeFound = false;
  int prefixLength = strlen(m_urlPrefix);

  if (strncmp(m_urlPrefix, request.urlPath(), prefixLength) == 0) {
    char * trimmedPath = request.urlPath() + prefixLength;

    CommandNode * command = m_tailCommand;

    while (command != NULL && !response.ended()) {
      if (command->type == request.method()
          || command->type == Request::ALL
          || command->type == Request::USE) {

        if (command->type == Request::USE
            || m_routeMatch(trimmedPath,
                            command->urlPattern)) {

          if (command->type != Request::USE) {
            routeFound = true;
          }

          request.m_setRoute(prefixLength, command->urlPattern);
          command->command(request, response);
        }
      }

      command = command->next;
    }
  }

  return routeFound;
}

/* Mounts a middleware command to the router which is executed when a HTTP request with method type GET matches the url pattern. */
void Router::get(const char *urlPattern, Middleware *command) {
  m_addCommand(Request::GET, urlPattern, command);
}

/* Mounts a middleware command to the router which is executed when a HTTP request with method type POST matches the url pattern. */
void Router::post(const char *urlPattern, Middleware *command) {
  m_addCommand(Request::POST, urlPattern, command);
}

/* Mounts a middleware command to the router which is executed when a HTTP request with method type PUT matches the url pattern. */
void Router::put(const char *urlPattern, Middleware *command) {
  m_addCommand(Request::PUT, urlPattern, command);
}

/* Mounts a middleware command to the router which is executed when a HTTP request with method type DELETE matches the url pattern. */
void Router::del(const char *urlPattern, Middleware *command) {
  m_addCommand(Request::DELETE, urlPattern, command);
}

/* Mounts a middleware command to the router which is executed when a HTTP request with method type PATCH matches the url pattern. */
void Router::patch(const char *urlPattern, Middleware *command) {
  m_addCommand(Request::PATCH, urlPattern, command);
}

/* Mounts a middleware command to the router which is executed when a HTTP request with method type OPTIONS matches the url pattern. */
void Router::options(const char *urlPattern, Middleware *command) {
  m_addCommand(Request::OPTIONS, urlPattern, command);
}

/* Mounts a middleware command to the router which is executed when a HTTP request regardless of method type matches the url pattern. */
void Router::all(const char *urlPattern, Middleware *command) {
  m_addCommand(Request::ALL, urlPattern, command);
}

/* Mounts a middleware command to be executed on every request regardless of method type and URL although possible router URL prefix has to match */
void Router::use(Middleware *command) {
  m_addCommand(Request::USE, NULL, command);
}

void Router::m_addCommand(Request::MethodType type, const char *urlPattern, Middleware *command) {
  CommandNode* newCommand = (CommandNode*) malloc(sizeof(CommandNode));

  newCommand->urlPattern = urlPattern;
  newCommand->command = command;
  newCommand->type = type;
  newCommand->next = NULL;

  if (m_tailCommand == NULL ) {
    m_tailCommand = newCommand;
  } else {
    CommandNode * commandNode = m_tailCommand;

    while (commandNode->next != NULL) {
      commandNode = commandNode->next;
    }

    commandNode->next = newCommand;
  }
}

Router * Router::m_getNext() {
  return m_next;
}

void Router::m_setNext(Router * next) {
  m_next = next;
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

  if (match && !pattern[i] && text[j] == '/' && !text[j + 1]) {
    match = true;
  } else if (pattern[i] || text[j]) {
    match = false;
  }

  return match;
}

/* Server class constructor. */
WebApp::WebApp() :
  m_clientObject(NULL),
  m_failureCommand(&m_defaultFailCommand),
  m_notFoundCommand(&m_defaultNotFoundCommand),
  m_headerTail(NULL) {
  m_routerTail = &m_defaultRouter;
}

/* Processes an incoming connection with default request buffer length. */
void WebApp::process(Client *client) {
  char request[SERVER_DEFAULT_REQUEST_LENGTH];
  process(client, request, SERVER_DEFAULT_REQUEST_LENGTH);
}

/* Processes an incoming connection with request buffer and length given as parameters. */
void WebApp::process(Client *client, char *buff, int bufflen) {
  m_clientObject = client;
  bool routeMatch = false;

  if (m_clientObject == NULL) {
    return;
  }

  m_request.m_init(m_clientObject, buff, bufflen);
  m_response.m_init(m_clientObject);
  m_request.m_processRequestLine();

  if (m_request.method() == Request::INVALID) {
    m_failureCommand(m_request, m_response);
  } else {
    m_request.m_decodeURL();
    m_request.m_processURL();
    m_request.m_processHeaders(m_headerTail);

    Router* routerNode = m_routerTail;

    while (routerNode != NULL && !m_response.ended()) {
      if (routerNode->m_dispatchCommands(m_request, m_response)) {
        routeMatch = true;
      }

      routerNode = routerNode->m_getNext();
    }

    if (!routeMatch && !m_response.ended()) {
      m_notFoundCommand(m_request, m_response);
    }
  }

  m_request.m_reset();
  m_response.m_reset();
}

/* Sets the default failure command for the server. Executed whem request is considered malformed. */
void WebApp::failCommand(Router::Middleware *command) {
  m_failureCommand = command;
}

/* Sets the default not found command for the server. Executed whem no routes match the query. */
void WebApp::notFoundCommand(Router::Middleware *command) {
  m_notFoundCommand = command;
}

/* Mounts a middleware command to the default router which is executed when a HTTP request with method type GET matches the url pattern. */
void WebApp::get(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.m_addCommand(Request::GET, urlPattern, command);
}

/* Mounts a middleware command to the default router which is executed when a HTTP request with method type POST matches the url pattern. */
void WebApp::post(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.m_addCommand(Request::POST, urlPattern, command);
}

/* Mounts a middleware command to the default router which is executed when a HTTP request with method type PUT matches the url pattern. */
void WebApp::put(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.m_addCommand(Request::PUT, urlPattern, command);
}

/* Mounts a middleware command to the default router which is executed when a HTTP request with method type DELETE matches the url pattern. */
void WebApp::del(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.m_addCommand(Request::DELETE, urlPattern, command);
}

/* Mounts a middleware command to the default router which is executed when a HTTP request with method type PATCH matches the url pattern. */
void WebApp::patch(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.m_addCommand(Request::PATCH, urlPattern, command);
}

/* Mounts a middleware command to the default router which is executed when a HTTP request with method type OPTIONS matches the url pattern. */
void WebApp::options(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.m_addCommand(Request::OPTIONS, urlPattern, command);
}

/* Mounts a middleware command to the default router which is executed when a HTTP request regardless of method type matches the url pattern. */
void WebApp::all(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.m_addCommand(Request::ALL, urlPattern, command);
}

/* Mounts a middleware command to be executed on every request regardless of method type and url */
void WebApp::use(Router::Middleware *command) {
  m_defaultRouter.m_addCommand(Request::USE, NULL, command);
}

/* Mounts a Router instance to the server. */
void WebApp::use(Router * router) {

  Router * routerNode = m_routerTail;

  while (routerNode->m_getNext() != NULL) {
    routerNode = routerNode->m_getNext();
  }

  routerNode->m_setNext(router);
}

void WebApp::readHeader(const char* name, char* buffer, int bufflen) {
  Request::HeaderNode* newNode = (Request::HeaderNode*) malloc(sizeof(Request::HeaderNode));

  newNode->name = name;
  newNode->buffer = buffer;
  newNode->size = bufflen;
  newNode->next = NULL;

  if (m_headerTail == NULL ) {
    m_headerTail = newNode;
  } else {
    Request::HeaderNode * headerNode = m_headerTail;

    while (headerNode->next != NULL) {
      headerNode = headerNode->next;
    }

    headerNode->next = newNode;
  }
}

/* Executed when request is considered malformed. */
void WebApp::m_defaultFailCommand(Request &request, Response &response) {
  response.fail();
}

/* Executed when no routes match the query. */
void WebApp::m_defaultNotFoundCommand(Request &request, Response &response) {
  response.notFound();
}
