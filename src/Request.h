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
  // Null for absent or uncaptured headers; present empty values return "".
  char* get(const char* name);
  int left();
  MethodType method();
  char* path();
  int peek();
  void push(uint8_t ch);
  // Raw encoded query (without ?); path() is percent-decoded.
  char* query();
  // Match decoded names and decode values; + is a space. Invalid escapes,
  // decoded NUL, and insufficient capacity fail without overrunning the buffer.
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
  Request(Client* client, Response* m_response, ArrayView<Header> headers,
          char* urlBuffer, int urlBufferLength, unsigned long timeout,
          void* context);
  bool m_processMethod();
  bool m_readURL();
  bool m_readVersion();
  bool m_processURL();
  int m_processHeaders();
  bool m_headerName(Header*& header);
  bool m_headerValue(char* buffer, int bufferLength, bool append);
  bool m_readInt(int& number);
  void m_setRoute(const char* route, const char* pattern);
  bool m_expect(const char* expected);
  bool m_expectP(const unsigned char* expected);
  bool m_skipSpace();
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
  ArrayView<Header> m_headers;
  char* m_query;
  bool m_readTimedout;
  char* m_path;
  int m_pathLength;
  const char* m_pattern;
  const char* m_route;
  bool m_next;
};

}  // namespace awot

#endif
