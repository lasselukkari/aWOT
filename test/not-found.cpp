#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request &req, Response &res) {
  res.set("Content-Type", "text/plain");
  res.print("/");
}

unittest(root_path) {
  const char *request =
    "GET /notfound HTTP/1.0" CRLF
    CRLF;

  const char *expected =
    "HTTP/1.1 404 Not Found" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "Not Found";

  MockStream stream(request);
  Application app;

  app.get("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
