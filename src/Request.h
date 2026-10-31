#ifndef AWOT_REQUEST_H_
#define AWOT_REQUEST_H_

#include "ArrayView.h"
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
  struct Header {
    Header(const char* name, ArrayView<char> buffer)
        : name(name), buffer(buffer), m_present(false) {}
    Header(const char* name, char* buffer, int bufferLength)
        : Header(name, ArrayView<char>(buffer, bufferLength)) {}

    const char* name;
    ArrayView<char> buffer;

   private:
    friend class App;
    friend class Request;
    bool m_present;
  };

  void* context;

  int available();
  int availableForWrite();
  int bytesRead();
  Client* client();
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
  enum BodyState : uint8_t {
    CONTENT_LENGTH,
    CHUNK_START,
    CHUNK_SIZE,
    CHUNK_EXTENSION_START,
    CHUNK_EXTENSION,
    CHUNK_SIZE_LF,
    CHUNK_DATA,
    CHUNK_DATA_CR,
    CHUNK_DATA_LF,
    TRAILER_START,
    TRAILER_NAME,
    TRAILER_VALUE,
    TRAILER_LF,
    BODY_END_LF,
    BODY_DONE,
    BODY_ERROR
  };

  Request(Client* client, Response* m_response, ArrayView<Header> headers,
          char* urlBuffer, int urlBufferLength, unsigned long timeout,
          void* context);
  bool m_processMethod();
  bool m_readURL();
  bool m_readVersion();
  bool m_processURL();
  int m_processHeaders();
  bool m_headerName(Header*& header, bool& connection);
  bool m_headerValue(Header* header, bool connection);
  bool m_allowsKeepAlive() const;
  bool m_readInt(int& number);
  void m_setRoute(const char* route, const char* pattern);
  bool m_expectP(const unsigned char* expected);
  bool m_skipSpace();
  int m_readRaw();
  bool m_bodyAvailable();
  int m_bodyError() const;
  int m_timedRead();
  bool m_timedout();

  Client* m_stream;
  Response* m_response;
  MethodType m_method;
  int m_minorVersion;
  bool m_connectionClose;
  bool m_connectionKeepAlive;
  unsigned char m_pushback[SERVER_PUSHBACK_BUFFER_SIZE];
  int m_pushbackDepth;
  bool m_readingContent;
  BodyState m_bodyState;
  uint32_t m_left;
  int m_bytesRead;
  ArrayView<Header> m_headers;
  char* m_query;
  bool m_readTimedout;
  char* m_path;
  int m_pathLength;
  const char* m_pattern;
  const char* m_route;
  bool m_next;
};

}

#endif
