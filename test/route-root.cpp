#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request & req, Response & res) {
  res.set("Content-Type", "text/plain");
  res.print("/");
}

unittest(root_path) {
  const char *request =
    "GET / HTTP/1.0" CRLF
    CRLF;

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "/";

  MockStream stream(request);
  Application app;

  app.get("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
