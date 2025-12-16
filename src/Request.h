/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things
*/

#ifndef AWOT_REQUEST_H_
#define AWOT_REQUEST_H_

#include "Response.h"

namespace awot {

class Request : public Stream {
  friend class App;
  friend class Router;

 public:
  enum MethodType {
    UNKNOWN,
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    PATCH,
    OPTIONS,
    ALL
  };
  struct HeaderNode {
    const char* name;
    char* buffer;
    int bufferLength;
    HeaderNode* next;
  };

  void* context;

  int available();
  int availableForWrite();
  int bytesRead();
  Stream* stream();
  void flush();
  bool form(char* name, int nameLength, char* value, int valueLength);
  char* get(const char* name);
  int left();
  MethodType method();
  char* path();
  int peek();
  void push(uint8_t ch);
  char* query();
  bool query(const char* name, char* buffer, int bufferLength);
  int read();
  int read(uint8_t* buf, size_t size);
  bool route(const char* name, char* buffer, int bufferLength);
  bool route(int number, char* buffer, int bufferLength);
  int minorVersion();
  void next();
  size_t write(uint8_t data);
  size_t write(const uint8_t* buffer, size_t bufferLength);

 private:
  Request(Client* client, Response* m_response, HeaderNode* headerTail,
          char* urlBuffer, int urlBufferLength, unsigned long timeout,
          void* context);
  bool m_processMethod();
  bool m_readURL();
  bool m_readVersion();
  void m_processURL();
  bool m_processHeaders();
  bool m_headerValue(char* buffer, int bufferLength);
  bool m_readInt(int& number);
  void m_setRoute(const char* route, const char* pattern);
  int m_getUrlPathLength();
  bool m_expect(const char* expected);
  bool m_expectP(const unsigned char* expected);
  bool m_skipSpace();
  void m_reset();
  int m_timedRead();
  bool m_timedout();

  Client* m_stream;
  Response* m_response;
  MethodType m_method;
  int m_minorVersion;
  unsigned char m_pushback[SERVER_PUSHBACK_BUFFER_SIZE];
  int m_pushbackDepth;
  bool m_readingContent;
  int m_left;
  int m_bytesRead;
  HeaderNode* m_headerTail;
  char* m_query;
  int m_queryLength;
  bool m_readTimedout;
  char* m_path;
  int m_pathLength;
  const char* m_pattern;
  const char* m_route;
  bool m_next;
};

}  // namespace awot

#endif
