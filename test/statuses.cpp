#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request & req, Response & res) {
  res.set("Before-Continue", "TestBefore");
  res.status(100);
  res.set("After-Continue", "TestAfter");
  res.sendStatus(204);
}

unittest(status_continue) {
  const char *request =
    "POST / HTTP/1.1" CRLF
    "Expect: 100-continue" CRLF
    "Content-Length: 4" CRLF
    CRLF
    "test";

  const char *expected =
    "HTTP/1.1 100 Continue" CRLF
    "Before-Continue: TestBefore" CRLF
    CRLF
    "HTTP/1.1 204 No Content" CRLF
    "Before-Continue: TestBefore" CRLF
    "After-Continue: TestAfter" CRLF
    "Connection: close" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
