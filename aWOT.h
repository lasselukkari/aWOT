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

#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include <string.h>
#include <stdlib.h>
#include <Arduino.h>

#include "Client.h"

#define CRLF "\r\n"

#ifndef SERVER_DEFAULT_REQUEST_LENGTH
#define SERVER_DEFAULT_REQUEST_LENGTH 64
#endif

#ifndef SERVER_OUTPUT_BUFFER_SIZE
#if defined(ESP8266) || defined (ESP32)
#define SERVER_OUTPUT_BUFFER_SIZE 1024
#else
#define SERVER_OUTPUT_BUFFER_SIZE 32
#endif
#endif

#ifndef SERVER_HEADERS_COUNT
#define SERVER_HEADERS_COUNT 5
#endif

#ifndef SERVER_READ_TIMEOUT_IN_MS
#define SERVER_READ_TIMEOUT_IN_MS 10000
#endif

#ifndef SERVER_FAIL_MESSAGE
#define SERVER_FAIL_MESSAGE "<h1>400 Bad Request</h1>"
#endif

#ifndef SERVER_NOT_FOUND_MESSAGE
#define SERVER_NOT_FOUND_MESSAGE "<h1>404 Not Found</h1>"
#endif

#ifndef SERVER_AUTH_MESSAGE
#define SERVER_AUTH_MESSAGE "<h1>401 Unauthorized</h1>"
#endif

#ifndef SERVER_FORBIDDEN_MESSAGE
#define SERVER_FORBIDDEN_MESSAGE "<h1>403 Forbidden</h1>"
#endif

#ifndef SERVER_SERVER_ERROR_MESSAGE
#define SERVER_SERVER_ERROR_MESSAGE "<h1>500 Internal Server Error</h1>"
#endif

#define P(name) static const unsigned char name[] PROGMEM

#define SIZE(array) (sizeof(array) / sizeof(*array))

#ifdef _VARIANT_ARDUINO_DUE_X_
#define pgm_read_byte(ptr) (unsigned char)(* ptr)
#endif

class Request: public Stream {
    friend class WebApp;
    friend class Router;

  public:
    enum MethodType {
      INVALID, GET, HEAD, POST, PUT, DELETE, PATCH, OPTIONS, ALL, USE
    };

    struct HeaderNode {
      const char* name;
      char* buffer;
      int size;
      HeaderNode* next;
    };

    Request();

    MethodType method();
    int contentLeft();

    char * urlPath();

    bool route(const char *key, char *paramBuffer, int paramBufferLen);
    bool route(int number, char *paramBuffer, int paramBufferLen);

    char * query();
    bool query(const char *key, char *paramBuffer, int paramBufferLen);
    bool queryComplete();

    bool postParam(char *name, int nameLen, char *value, int valueLen);

    char * header(const char *name);

    int available();
    int read();
    int bytesRead();
    int peek();
    void push(int ch);

    // dummy implementation to fulfill the Stream interface
    size_t write(uint8_t ch) {
      return 0;
    }
    void flush() {
      return;
    }

  private:
    void m_init(Client *client, char* buff, int bufflen);
    void m_processRequestLine();
    void m_processURL();
    void m_decodeURL();
    void m_processHeaders(HeaderNode* headerTail);
    void m_setRoute(int prefixLength, const char * routeString);
    int m_getUrlPathLength();
    void m_reset();

    void m_readHeader(char *value, int valueLen);
    bool m_readInt(int &number);
    bool m_expect(const char *expectedStr);

    Client * m_clientObject;
    MethodType m_methodType;
    unsigned char m_pushback[32];
    int m_pushbackDepth;
    bool m_readingContent;

    int m_contentLeft;
    int m_bytesRead;

    HeaderNode* m_headerTail;

    char * m_query;
    bool m_queryComplete;

    char * m_urlPath;
    int m_urlPathLength;
    int m_prefixLength;

    int m_hexToInt(char *hex);

    const char * m_route;

    bool m_next;

};

class Response: public Stream {
    friend class WebApp;
    friend class Router;

  public:
    Response();

    void printP(const unsigned char *str);
    void printP(const char *str) {
      printP((unsigned char*) str);
    }
    void writeP(const unsigned char *data, size_t length);
    size_t write(uint8_t ch);
    size_t write(uint8_t *ch, size_t size);
    void flush();
    int bytesSent();

    void end();
    bool ended();

    void set(const char* name, const char* value);

    void success(const char *contentType);
    void created(const char *contentType);
    void noContent();

    void seeOther(const char *otherURL);
    void notModified();

    void fail();
    void unauthorized();
    void forbidden();
    void notFound();
    void serverError();

    //dummy implementations to fulfill Stream interface
    int available() {
      return 0;
    }
    int read() {
      return -1;
    }
    int peek() {
      return -1;
    }

  private:

    void m_init(Client *client);
    void m_reset();
    void m_printCRLF();
    void m_printHeaders();
    void m_flushBuf();

    Client * m_clientObject;

    struct Headers {
      const char* name;
      const char* value;
    } m_headers[SERVER_HEADERS_COUNT];

    unsigned int m_headersCount;
    int m_bytesSent;
    bool m_ended;
    uint8_t m_buffer[SERVER_OUTPUT_BUFFER_SIZE];
    int m_bufFill;
};

class Router {
    friend class WebApp;

  public:
    typedef void Middleware(Request& request, Response& response);

    Router(const char * urlPrefix = "");

    void get(const char* urlPattern, Middleware* command);
    void post(const char* urlPattern, Middleware* command);
    void put(const char* urlPattern, Middleware* command);
    void del(const char* urlPattern, Middleware* command);
    void patch(const char* urlPattern, Middleware* command);
    void options(const char* urlPattern, Middleware* command);
    void all(const char* urlPattern, Middleware* command);
    void use(Middleware* command);

  private:
    struct CommandNode {
      const char* urlPattern;
      Middleware* command;
      Request::MethodType type;
      CommandNode* next;
    };

    bool m_dispatchCommands(Request& request, Response& response);
    bool m_routeMatch(const char *str, const char *pattern);
    void m_addCommand(Request::MethodType type, const char* urlPattern, Middleware* command);
    Router * m_getNext();
    void m_setNext(Router * next);

    CommandNode* m_tailCommand;
    Router* m_next;
    const char * m_urlPrefix;

};

class WebApp {

  public:
    WebApp();

    void process(Client *client);
    void process(Client *client, char* buff, int bufflen);

    void failCommand(Router::Middleware* command);
    void notFoundCommand(Router::Middleware* command);

    void get(const char* urlPattern, Router::Middleware* command);
    void post(const char* urlPattern, Router::Middleware* command);
    void put(const char* urlPattern, Router::Middleware* command);
    void del(const char* urlPattern, Router::Middleware* command);
    void patch(const char* urlPattern, Router::Middleware* command);
    void options(const char* urlPattern, Router::Middleware* command);
    void all(const char* urlPattern, Router::Middleware* command);
    void use(Router::Middleware* command);
    void use(Router * router);
    void readHeader(const char* name, char* buffer, int bufflen);

  private:

    static void m_defaultFailCommand(Request &request, Response &response);
    static void m_defaultNotFoundCommand(Request &request, Response &response);

    Client * m_clientObject;

    Request m_request;
    Response m_response;

    Router * m_routerTail;
    Router m_defaultRouter;

    Router::Middleware* m_failureCommand;
    Router::Middleware* m_notFoundCommand;

    Request::HeaderNode* m_headerTail;

};

#endif
