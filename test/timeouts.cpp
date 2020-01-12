#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request &req, Response &res) {
  if(req.left() && req.read() == -1){
    res.sendStatus(408);
    return;
  }

  res.set("Content-Type", "text/plain");
  res.print("/");
}

char const *expected =
  "HTTP/1.1 408 Request Timeout" CRLF
  "Content-Type: text/plain" CRLF
  "Connection: close" CRLF
  CRLF
  "Request Timeout";

unittest(timeout_during_verb) {
  char const *request =
    "PO";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_after_verb) {
  char const *request =
    "POST";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_durin_url) {
  char const *request =
    "POST /";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_after_url) {
  char const *request =
    "POST / ";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_during_http) {
  char const *request =
    "POST / HT";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_after_http) {
  char const *request =
    "POST / HTTP";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_during_slash) {
  char const *request =
    "POST / HTTP/";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_during_major_version) {
  char const *request =
    "POST / HTTP/1";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_during_version_dot) {
  char const *request =
    "POST / HTTP/1.";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_during_version_minor) {
  char const *request =
    "POST / HTTP/1.0";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_during_linebreak) {
  char const *request =
    "POST / HTTP/1.0" CRLF;

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_during_header_name) {
  char const *request =
    "POST / HTTP/1.0" CRLF
    "Conten";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_after_header_name) {
  char const *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_during_semicolon) {
  char const *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length:";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_after_semicolon) {
  char const *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: ";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_during_header_value) {
  char const *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 1";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_after_header_value) {
  char const *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 1" CRLF;

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(timeout_during_body) {
  char const *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 1" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(no_timeout) {
  char const *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 1" CRLF
    CRLF
    "1";

  char const *ok =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "/";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(ok, stream.response());
}

unittest_main()
