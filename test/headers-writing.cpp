#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void customHeadersHandler(Request & req, Response & res) {
  res.set("Test1", "test1");
  res.set("Test1", "test2");
  res.print("/");
}

void contentTypeHandler(Request & req, Response & res) {
  res.set("Content-Type", "test/type");
  res.print("/");
}

void keepAliveHandler(Request & req, Response & res) {
  res.set("Connection", "keep-alive");
  res.print("/");
}

void keepAliveContentLengthHandler(Request & req, Response & res) {
  res.set("Connection", "keep-alive");
  res.set("Content-Length", "1");
  res.print("/");
}

unittest(custom_headers) {
  char const *request =
    "GET / HTTP/1.0" CRLF
    CRLF;

  char const *expected =
    "HTTP/1.1 200 OK" CRLF
    "Test1: test1" CRLF
    "Test1: test2" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "/";

  MockStream stream(request);
  Application app;

  app.get("/", &customHeadersHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(content_type) {
  char const *request =
    "GET / HTTP/1.0" CRLF
    CRLF;

  char const *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: test/type" CRLF
    "Connection: close" CRLF
    CRLF
    "/";

  MockStream stream(request);
  Application app;

  app.get("/", &contentTypeHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(connection_keep_alive) {
  char const *request =
    "GET / HTTP/1.0" CRLF
    CRLF
    "GET / HTTP/1.0" CRLF
    CRLF;

  char const *expected =
    "HTTP/1.1 200 OK" CRLF
    "Connection: keep-alive" CRLF
    "Content-Type: text/plain" CRLF
    "Transfer-Encoding: chunked" CRLF
    CRLF
    "1" CRLF
    "/" CRLF
    "0" CRLF
    CRLF
    "HTTP/1.1 200 OK" CRLF
    "Connection: keep-alive" CRLF
    "Content-Type: text/plain" CRLF
    "Transfer-Encoding: chunked" CRLF
    CRLF
    "1" CRLF
    "/" CRLF
    "0" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.get("/", &keepAliveHandler);
  app.process(&stream);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(connection_keep_alive_content_length) {
  char const *request =
    "GET / HTTP/1.0" CRLF
    CRLF;

  char const *expected =
    "HTTP/1.1 200 OK" CRLF
    "Connection: keep-alive" CRLF
    "Content-Length: 1" CRLF
    "Content-Type: text/plain" CRLF
    CRLF
    "/";

  MockStream stream(request);
  Application app;

  app.get("/", &keepAliveContentLengthHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
