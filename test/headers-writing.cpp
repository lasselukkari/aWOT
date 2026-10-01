#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"
#include <string.h>

void customHeadersHandler(Request & req, Response & res) {
  res.set("Test1", "test1");
  res.set("Test1", "test2");
  res.print("/");
}

void manualHeadersHandler(Request & req, Response & res) {
  res.set("Test1", "test1");
  res.setDefaults();
  res.beginHeaders();
  res.print("Test1: test2" CRLF);
  res.endHeaders();
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

void responseHeaderLookupHandler(Request & req, Response & res) {
  res.set("X-Test", "first");
  res.set("X-Other", "second");
  res.print(res.get("x-test"));
  res.print("/");
  res.print(res.get("X-Other"));
}

void customOutputBufferHandler(Request & req, Response & res) {
  res.print("buffer length is respected");
}

unittest(custom_headers) {
  const char *request =
    "GET / HTTP/1.0" CRLF
    CRLF;

  const char *expected =
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

unittest(response_header_lookup) {
  const char *request =
    "GET / HTTP/1.0" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.get("/", &responseHeaderLookupHandler);
  app.process(&stream);

  assertTrue(strstr(stream.response(), "first/second") != NULL);
}

unittest(custom_output_buffer_length) {
  const char *request =
    "GET / HTTP/1.0" CRLF
    CRLF;
  char urlBuffer[SERVER_URL_BUFFER_SIZE];
  struct {
    uint8_t outputBuffer[4];
    uint8_t canary[8];
  } guardedBuffer;
  memset(guardedBuffer.canary, 0xA5, sizeof(guardedBuffer.canary));
  MockStream stream(request);
  Application app;

  app.get("/", &customOutputBufferHandler);
  app.process(&stream, urlBuffer, sizeof(urlBuffer), guardedBuffer.outputBuffer,
              sizeof(guardedBuffer.outputBuffer));

  for (size_t i = 0; i < sizeof(guardedBuffer.canary); i++) {
    assertEqual((uint8_t)0xA5, guardedBuffer.canary[i]);
  }
  assertTrue(strstr(stream.response(), "buffer length is respected") != NULL);
}

unittest(manual_headers) {
  const char *request =
    "GET / HTTP/1.0" CRLF
    CRLF;

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Test1: test1" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    "Test1: test2" CRLF
    CRLF
    "/";

  MockStream stream(request);
  Application app;

  app.get("/", &manualHeadersHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(content_type) {
  const char *request =
    "GET / HTTP/1.0" CRLF
    CRLF;

  const char *expected =
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
  const char *request =
    "GET / HTTP/1.0" CRLF
    CRLF
    "GET / HTTP/1.0" CRLF
    CRLF;

  const char *expected =
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
  const char *request =
    "GET / HTTP/1.0" CRLF
    CRLF;

  const char *expected =
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
