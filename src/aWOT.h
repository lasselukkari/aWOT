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

#ifndef AWOT_H_
#define AWOT_H_

#include <string.h>
#include <stdlib.h>
#include <Arduino.h>

#include "Client.h"

#define CRLF "\r\n"

#if defined(__AVR_ATmega328P__) || defined (__AVR_Atmega32U4__) || defined (__AVR_ATmega16U4__) || defined (_AVR_ATmega328__)
#define LOW_SRAM_MCU
#endif

#ifndef SERVER_URL_BUFFER_SIZE
#if defined(LOW_SRAM_MCU)
#define SERVER_URL_BUFFER_SIZE 64
#else
#define SERVER_URL_BUFFER_SIZE 256
#endif
#endif

#ifndef SERVER_PUSHBACK_BUFFER_SIZE
#if defined(LOW_SRAM_MCU)
#define SERVER_PUSHBACK_BUFFER_SIZE 32
#else
#define SERVER_PUSHBACK_BUFFER_SIZE 128
#endif
#endif

#ifndef SERVER_OUTPUT_BUFFER_SIZE
#if defined(LOW_SRAM_MCU)
#define SERVER_OUTPUT_BUFFER_SIZE 32
#else
#define SERVER_OUTPUT_BUFFER_SIZE 1024
#endif
#endif

#ifndef SERVER_MAX_HEADERS
#define SERVER_MAX_HEADERS 10
#endif

#ifndef SERVER_READ_TIMEOUT_MS
#define SERVER_READ_TIMEOUT_MS 1000
#endif

#ifdef _VARIANT_ARDUINO_DUE_X_
#define pgm_read_byte(ptr) (unsigned char)(* ptr)
#endif

#define P(name) static const unsigned char name[] PROGMEM
#define SIZE(array) (sizeof(array) / sizeof(*array))

class Request: public Stream {
    friend class Application;
    friend class Router;

  public:
    enum MethodType {
      GET, HEAD, POST, PUT, DELETE, PATCH, OPTIONS, ALL, USE
    };

    int available();
    bool body(uint8_t *buffer, int bufferLength);
    int bytesRead();
    Client * client();
    bool form(char *name, int nameLength, char *value, int valueLength);
    char * get(const char *name);
    int left();
    MethodType method();
    char * path();
    int peek();
    void push(uint8_t ch);
    char * query();
    bool query(const char *name, char *buffer, int bufferLength);
    int read();
    bool route(const char *name, char *buffer, int bufferLength);
    bool route(int number, char *buffer, int bufferLength);
    bool timeout();

    // dummy implementation for the stream intreface
    void flush();
    size_t write(uint8_t data);

  private:
    struct HeaderNode {
      const char* name;
      char* buffer;
      int bufferLength;
      HeaderNode* next;
    };

    Request();
    void m_init(Client *client, char* buffer, int bufferLength);
    bool m_processMethod();
    bool m_readURL();
    void m_processURL();
    bool m_processHeaders(HeaderNode* headerTail);
    bool m_headerValue(char *buffer, int bufferLength);
    bool m_readInt(int &number);
    void m_setRoute(int prefixLength, const char * route);
    void m_setMethod(MethodType method);
    int m_getUrlPathLength();
    bool m_expect(const char *expected);
    void m_reset();

    Client * m_clientObject;
    MethodType m_method;
    unsigned char m_pushback[SERVER_PUSHBACK_BUFFER_SIZE];
    int m_pushbackDepth;
    bool m_readingContent;
    int m_left;
    int m_bytesRead;
    HeaderNode* m_headerTail;
    char * m_query;
    int m_queryLength;
    bool m_timeout;
    char * m_path;
    int m_pathLength;
    int m_prefixLength;
    const char * m_route;
    bool m_next;
};

class Response: public Stream {
    friend class Application;
    friend class Router;

  public:
    int bytesSent();
    void end();
    bool ended();
    void flush();
    const char * get(const char *name);
    bool headersSent();
    void printP(const unsigned char *string);
    void printP(const char *string);
    void sendStatus(int code);
    void set(const char* name, const char* value);
    void status(int code);
    bool statusSent();
    size_t write(uint8_t data);
    size_t write(uint8_t *buffer, size_t bufferLength);
    void writeP(const unsigned char *data, size_t length);

    // dummy implementation for the stream intreface
    int available();
    int peek();
    int read();

  private:
    Response();

    void m_init(Client *client);
    void m_printStatus(int code);
    bool m_shouldPrintHeaders();
    void m_printHeaders();
    void m_printCRLF();
    void m_flushBuf();
    void m_reset();

    Client * m_clientObject;
    struct Headers {
      const char* name;
      const char* value;
    } m_headers[SERVER_MAX_HEADERS];
    bool m_contentTypeSet;
    bool m_statusSent;
    bool m_headersSent;
    bool m_sendingStatus;
    bool m_sendingHeaders;
    unsigned int m_headersCount;
    char* m_mime;
    int m_bytesSent;
    bool m_ended;
    uint8_t m_buffer[SERVER_OUTPUT_BUFFER_SIZE];
    int m_bufFill;
};

class Router {
    friend class Application;

  public:
    typedef void Middleware(Request& request, Response& response);

    Router(const char * urlPrefix = "");

    void all(const char* path, Middleware* middleware);
    void del(const char* path, Middleware* middleware);
    void get(const char* path, Middleware* middleware);
    void options(const char* path, Middleware* middleware);
    void patch(const char* path, Middleware* middleware);
    void post(const char* path, Middleware* middleware);
    void put(const char* path, Middleware* middleware);
    void use(Middleware* middleware);

  private:
    struct MiddlewareNode {
      const char* path;
      Middleware* middleware;
      Request::MethodType type;
      MiddlewareNode* next;
    };

    void m_addMiddleware(Request::MethodType type, const char* path, Middleware* middleware);
    void m_setNext(Router * next);
    Router * m_getNext();
    bool m_dispatchMiddleware(Request& request, Response& response);
    bool m_routeMatch(const char *route, const char *pattern);

    MiddlewareNode* m_head;
    Router* m_next;
    const char * m_urlPrefix;
};

class Application {
  public:
    Application();

    static int strcmpi(const char *s1, const char *s2);

    void all(const char* path, Router::Middleware* middleware);
    void del(const char* path, Router::Middleware* middleware);
    void get(const char* path, Router::Middleware* middleware);
    void header(const char* name, char* buffer, int bufferLength);
    void options(const char* path, Router::Middleware* middleware);
    void patch(const char* path, Router::Middleware* middleware);
    void post(const char* path, Router::Middleware* middleware);
    void put(const char* path, Router::Middleware* middleware);
    void process(Client *client);
    void process(Client *client, char* buffer, int bufferLength);
    void route(Router * router);
    void use(Router::Middleware* middleware);

  private:
    void m_process();

    Client * m_clientObject;
    Request m_request;
    Response m_response;
    Router * m_routerTail;
    Router m_defaultRouter;
    Request::HeaderNode* m_headerTail;
};

#endif
