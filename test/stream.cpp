#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void availableHandler(Request &req, Response &res) {
  req.setTimeout(10);
  res.print(req.available());
}

void readStringHandler(Request &req, Response &res) {
  res.print(req.readString());
}

void readBytesHandler(Request &req, Response &res) {
  char type[5] = {};
  req.readBytes(type, 5);
  res.print(type);
}

void reqPrintHandler(Request &req, Response &res) {
  req.print("TEST");
}

unittest(req_available) {
  const char *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 4" CRLF
    CRLF
    "test"
    "GET / HTT...";

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "4";

  MockStream stream(request);
  Application app;

  app.post("/", &availableHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(req_available_less_than_content_length) {
  const char *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 4" CRLF
    CRLF
    "te";

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "2";

  MockStream stream(request);
  Application app;

  app.post("/", &availableHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(req_read_string) {
  const char *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 4" CRLF
    CRLF
    "test"
    "GET / HTT...";

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "test";

  MockStream stream(request);
  Application app;

  app.post("/", &readStringHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(req_read_bytes) {
  const char *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 4" CRLF
    CRLF
    "test"
    "GET / HTT...";

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "test";

  MockStream stream(request);
  Application app;

  app.post("/", &readBytesHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(req_read_bytes_timeout) {
  const char *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 4" CRLF
    CRLF
    "te";

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "te";

  MockStream stream(request);
  Application app;

  app.post("/", &readBytesHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(req_print) {
  const char *request =
    "GET / HTTP/1.0" CRLF
    CRLF;

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "TEST";

  MockStream stream(request);
  Application app;

  app.get("/", &reqPrintHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
