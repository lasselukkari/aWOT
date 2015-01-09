/*
 aWOT, Express.js inspired  microcontreller web framework for the Web of Things
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

#include <arduino.h>
#include <aWOT.h>

/* Request constructor. */
Request::Request() :
  m_queryComplete(false),
  m_readingContent(false),
  m_next(true),
  m_urlPathPartsCount(0),
  m_urlPathLength(0),
  m_route(NULL),
  m_clientObject(NULL),
  m_urlPath(NULL),
  m_query(NULL),
  m_methodType(INVALID),
  m_pushbackDepth(0),
  m_contentLeft(NULL) {
}

/* Initializes the request instance ready to process the incoming HTTP request. */
void Request::init(Client *client, char* buff, int bufflen) 
{
  m_clientObject = client;
  m_urlPath = buff;
  m_urlPathLength = bufflen - 1;
}

/* Processes the first line of the HTTP request to parse method, verb and query. */
void Request::processRequest() {

  char * qmLocation;
  int qmOffset;
  char * request = m_urlPath;
  int bufferLeft = m_urlPathLength;

  // store the HTTP method line of the request
  if (m_expect("GET ")) {
    m_methodType = GET;
  }

  else if (m_expect("HEAD ")) {
    m_methodType = HEAD;
  }

  else if (m_expect("POST ")) {
    m_methodType = POST;
  }

  else if (m_expect("PUT ")) {
    m_methodType = PUT;
  }

  else if (m_expect("DELETE ")) {
    m_methodType = DELETE;
  }

  else if (m_expect("PATCH ")) {
    m_methodType = PATCH;
  }

  else {
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

  if (m_urlPath[0] == '/') {

    m_urlPath++;
    m_queryComplete = (bufferLeft);

    qmLocation = strchr(m_urlPath, '?');
    qmOffset = (qmLocation == NULL) ? 0 : 1;

    m_urlPathLength =
        (qmLocation == NULL) ? strlen(m_urlPath) : (qmLocation - m_urlPath);
    m_query = m_urlPath + m_urlPathLength + qmOffset;

    if (qmOffset) {
      *qmLocation = 0;
    }

  }

}

/* Processes the header fields of the request */
void Request::processHeaders() {

  m_basicAuth[0] = 0;
  m_cookie[0] = 0;

  while (1) {

    if (m_expect("Content-Length:")) {
      m_readInt(m_contentLeft);
      continue;
    }

    if (m_expect("Authorization:")) {
      m_readHeader(m_basicAuth, 51);
      continue;
    }

    if (m_expect("Cookie:")) {
      m_readHeader(m_cookie, 51);
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

bool Request::next() {
  return m_next;
}

/* Prevents any remaining Middleware commands to be executed after calling the method */
void Request::discontinue() {
  m_next = false;
}

char * Request::urlPath() {
  return m_urlPath;
}

int Request::urlPathLength() {
  return m_urlPathLength;
}

void Request::routeString(const char * routeString) {
  m_route = routeString;
}

/* Returns the URL path as an array of strings sliced in parts by "/". */
char ** Request::route() {
  return m_urlPathParts;
}

/* Returns a single route parameter by name. For example with route users/:userId and request to URL /users/123 request.route("userId") would return a char pointer to "123" */
char * Request::route(char *name) {

  byte part = 0;
  byte i = 0;

  bool found = true;

  while (m_route[i]) {

    if (m_route[i] == ':') {

      byte j = 0;
      i++; //skip to the next char after :

      while ((m_route[i] && name[j]) && m_route[i++] == name[j++]  )

      if (!name[j] && (m_route[i] == '/' || !m_route[i])) {

        return m_urlPathParts[part];

      }

    }

    if (m_route[i] == '/') {
      part++;
    }

    i++;

  }

  return NULL;

}

/* Returns a single route parameter by positions in the path. For example with  request to URL users/123 request.route(0)
 * would return a char pointer to "users" */
char * Request::route(int number) {

  if (number <= m_urlPathPartsCount){
      return m_urlPathParts[number];
  } else {
    return NULL;
  } 

}

/* Return a char pointer to the request query placed after the ? character in the URL */
char * Request::query() {
  return m_query;
}

/* Returns a single query parameter by name. For example with  request to URL /search?query=word request.query("query")
 * would return a char pointer to "word" */
char * Request::query(char * key) {

  int charsRead = 0;

  char *pos1 = strstr(m_query, key); 

  if (pos1) { 

    pos1 += strlen(key); 

    if (*pos1 == '=') { 

      pos1++; 

      while (*pos1 && *pos1 != '&' && charsRead < SERVER_PARAM_LENGTH) { 

        if (*pos1 == '%') { 

          char hex[3] = { pos1[1], pos1[2], 0 };
          m_paramBuffer[charsRead++] = strtoul(hex, NULL, 16);
          pos1 += 3; 

        } else if( *pos1=='+' ) { 

          m_paramBuffer[charsRead++] = ' '; 
          pos1++; 

        } else { 

          m_paramBuffer[charsRead++] = *pos1++; 

        }

      } 

      m_paramBuffer[charsRead]= '\0';

      return m_paramBuffer;

    } 

  } 

  return NULL;

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

      int ch1 = read();
      int ch2 = read();

      if (ch1 == -1 || ch2 == -1) {
        return false;
      }

      char hex[3] = { ch1, ch2, 0 };
      ch = strtoul(hex, NULL, 16);

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

/* Returns a pointer to the  "Authorization" header value */
char * Request::basicAuth() {
  return m_basicAuth;
}

/* Returns a pointer to the  "Cookie" header value */
char * Request::cookie() {
  return m_cookie;
}

void Request::slicePath(int prefixLength) {

  m_urlPathPartsCount = 0;
  m_urlPathParts[m_urlPathPartsCount] = m_urlPath + prefixLength;
  m_urlPathPartsCount++;

  for (char * p = m_urlPath + prefixLength; p < m_urlPath + m_urlPathLength; p++) {

    if (*p == '/') {

      *p = 0;
      m_urlPathParts[m_urlPathPartsCount++] = p + 1;

      if (m_urlPathPartsCount == SERVER_URL_PATH_COMMAND_LENGTH) {
        break;
      }

    }

  }

}

void Request::unSlicePath(int prefixLength) {

  for (char * p = m_urlPath + prefixLength; p < m_urlPath + m_urlPathLength-prefixLength; p++) {

    if (*p == 0) {
      *p = '/';
    }

  }

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

        if (m_readingContent) {
          --m_contentLeft;
        }

        return ch;

      }

      else {

        unsigned long now = millis();

        if (now > timeoutTime) {
          reset();
          return -1;
        }

      }

    }

    return -1;

  }

  else {
    return m_pushback[--m_pushbackDepth];
  }

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

void Request::flush() {
  m_clientObject->flush();
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

void Request::reset() {

  m_clientObject->flush();
  m_clientObject->stop();

  m_pushbackDepth = 0;
  m_contentLeft = 0;
  m_readingContent = false;
  m_methodType = INVALID;
  m_next = true;

}

void Request::m_readHeader(char *value, int valueLen) {

  int ch;
  memset(value, 0, valueLen);
  --valueLen;

  do {
    ch = read();
  }
  while (ch == ' ' || ch == '\t');

  do {

    if (valueLen > 1) {
      *value++ = ch;
      --valueLen;
    }

    ch = read();

  }
  while (ch != '\r');

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
  }
  while (ch == ' ' || ch == '\t');

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

/* Request class constructor. */
Response::Response() :
  m_clientObject(NULL),
  m_headersCount(0) {
}

/* Initializes the request instance ready for outputting the HTTP response. */
void Response::init(Client *client) {
  m_clientObject = client;
}

void Response::writeP(const prog_uchar *data, size_t length) {

  uint8_t buffer[32];
  size_t bufferEnd = 0;

  while (length--) {

    if (bufferEnd == 32) {
      m_clientObject->write(buffer, 32);
      bufferEnd = 0;
    }

    buffer[bufferEnd++] = pgm_read_byte(data++);

  }

  if (bufferEnd > 0) {
    m_clientObject->write(buffer, bufferEnd);
  }

}

void Response::printP(const prog_uchar *str) {

  uint8_t buffer[32];
  size_t bufferEnd = 0;

  while (buffer[bufferEnd++] = pgm_read_byte(str++)) {
    if (bufferEnd == 32) {
      m_clientObject->write(buffer, 32);
      bufferEnd = 0;
    }
  }

  // write out everything left but trailing NUL
  if (bufferEnd > 1) {
    m_clientObject->write(buffer, bufferEnd - 1);
  }

}

size_t Response::write(uint8_t ch) {
  return m_clientObject->write(ch);
}

/* Sets a header name and value pair to the response. */
void Response::set(char *name, char *value) {

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
  "Access-Control-Allow-Origin: *" CRLF
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
  "Access-Control-Allow-Origin: *" CRLF
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

void Response::reset() {
  m_headersCount = 0;
}

void Response::m_printCRLF() {
  m_clientObject->write((const uint8_t *) "\r\n", 2);
}

void Response::m_printHeaders() {

  for (byte i = 0; i < m_headersCount; i++) {
    print(m_headers[i].name);
    print(": ");
    print(m_headers[i].value);
    m_printCRLF();
  }

}

/* Router class constructor with an optional URL prefix parameter */
Router::Router(const char * urlPrefix) :
  m_commandCount(0),
  m_urlPrefix(urlPrefix) {
}

bool Router::dispatchCommands(Request& request, Response& response) {

  bool routeFound = false;
  byte prefixLength = strlen(m_urlPrefix);

  if (strncmp(m_urlPrefix, request.urlPath(), prefixLength) == 0) {

    char * trimmedPath = request.urlPath() + prefixLength;

    if (trimmedPath[0]=='/'){
      trimmedPath++;
      prefixLength++;
    }

    for (byte i = 0; i < m_commandCount && request.next(); ++i) {
      if (m_commands[i].type == request.method()
          || m_commands[i].type == Request::ALL
          || m_commands[i].type == Request::USE) {

        if (m_commands[i].type == Request::USE
            || m_routeMatch(trimmedPath,
                m_commands[i].urlPattern)) {

          if (m_commands[i].type != Request::USE) {
            routeFound = true;
          }

          request.routeString(m_commands[i].urlPattern);
          request.slicePath(prefixLength);
          m_commands[i].command(request, response);
          request.unSlicePath(prefixLength);


        }
      }
    }
  }

  return routeFound;
}

/* Mounts a middleware command to the router which is executed when a HTTP request with method type GET matches the url pattern. */
void Router::get(const char *urlPattern, Middleware *command) {
  addCommand(Request::GET, urlPattern, command);
}

/* Mounts a middleware command to the router which is executed when a HTTP request with method type POST matches the url pattern. */
void Router::post(const char *urlPattern, Middleware *command) {
  addCommand(Request::POST, urlPattern, command);
}

/* Mounts a middleware command to the router which is executed when a HTTP request with method type PUT matches the url pattern. */
void Router::put(const char *urlPattern, Middleware *command) {
  addCommand(Request::PUT, urlPattern, command);
}

/* Mounts a middleware command to the router which is executed when a HTTP request with method type DELETE matches the url pattern. */
void Router::del(const char *urlPattern, Middleware *command) {
  addCommand(Request::DELETE, urlPattern, command);
}

/* Mounts a middleware command to the router which is executed when a HTTP request with method type PATCH matches the url pattern. */
void Router::patch(const char *urlPattern, Middleware *command) {
  addCommand(Request::PATCH, urlPattern, command);
}

/* Mounts a middleware command to the router which is executed when a HTTP request regardless of method type matches the url pattern. */
void Router::all(const char *urlPattern, Middleware *command) {
  addCommand(Request::ALL, urlPattern, command);
}

/* Mounts a middleware command to be executed on every request regardless of method type and URL although possible router URL prefix has to match */
void Router::use(Middleware *command) {
  addCommand(Request::USE, NULL, command);
}

void Router::addCommand(Request::MethodType type, const char *urlPattern,
    Middleware *command) {

  if (m_commandCount < SERVER_COMMANDS_LENGTH) {
    m_commands[m_commandCount].urlPattern = urlPattern;
    m_commands[m_commandCount].command = command;
    m_commands[m_commandCount].type = type;
    m_commandCount++;
  }

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
    }

    else if (pattern[i] == text[j]) {
      j++;
      i++;
      match = true;
    }
    else {
      match = false;
      break;
    }
  }

  if (match && !pattern[i] && text[j] == '/' && !text[j + 1]) {
    match = true;
  }
  else if (pattern[i] || text[j]) {
    match = false;
  }

  return match;

}

/* Server class constructor. */

WebApp::WebApp() :
  m_clientObject(NULL),
  m_routerCount(1),
  m_failureCommand(&m_defaultFailCommand),
  m_notFoundCommand(&m_defaultNotFoundCommand) {
  m_routers[0] = &m_defaultRouter;
}

/* Processes an incoming connection with default request buffer length. */
void WebApp::process(Client *client) {
  char request[SERVER_DEFAULT_REQUEST_LENGTH];
  process(client, request, SERVER_DEFAULT_REQUEST_LENGTH);
}

/* Processes an incoming connection with request buffer and length given as parameters. */
void WebApp::process(Client *client, char *buff,
    int bufflen) {

  m_clientObject = client;

  bool routeMatch = false;

  if (m_clientObject != NULL) {

    m_request.init(m_clientObject, buff, bufflen);
    m_response.init(m_clientObject);
    m_request.processRequest();

    if (m_request.method() == Request::INVALID) {
      m_failureCommand(m_request, m_response);
    } else {

      m_request.processHeaders();

      for (byte i = 0; i < m_routerCount; i++) {
        if (m_routers[i]->dispatchCommands(m_request, m_response)) {
          routeMatch = true;
        }
      }

      if (!routeMatch) {
        m_notFoundCommand(m_request, m_response);
      }

    }

    m_request.reset();
    m_response.reset();

  }

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
  m_defaultRouter.addCommand(Request::GET, urlPattern, command);
}

/* Mounts a middleware command to the default router which is executed when a HTTP request with method type POST matches the url pattern. */
void WebApp::post(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.addCommand(Request::POST, urlPattern, command);
}

/* Mounts a middleware command to the default router which is executed when a HTTP request with method type PUT matches the url pattern. */
void WebApp::put(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.addCommand(Request::PUT, urlPattern, command);
}

/* Mounts a middleware command to the default router which is executed when a HTTP request with method type DELETE matches the url pattern. */
void WebApp::del(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.addCommand(Request::DELETE, urlPattern, command);
}

/* Mounts a middleware command to the default router which is executed when a HTTP request with method type POST matches the url pattern. */
void WebApp::patch(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.addCommand(Request::PATCH, urlPattern, command);
}

/* Mounts a middleware command to the default router which is executed when a HTTP request regardless of method type matches the url pattern. */
void WebApp::all(const char *urlPattern, Router::Middleware *command) {
  m_defaultRouter.addCommand(Request::ALL, urlPattern, command);
}

/* Mounts a middleware command to be executed on every request regardless of method type and url */
void WebApp::use(Router::Middleware *command) {
  m_defaultRouter.addCommand(Request::USE, NULL, command);
}

/* Mounts a Router instance to the server. */

void WebApp::use(Router * router) {
  if (m_routerCount < SERVER_ROUTERS_COUNT) {
    m_routers[m_routerCount++] = router;
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

