#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request &req, Response &res) {
  res.print(req.available());
}

unittest(req_available) {
  char const *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 4" CRLF
    CRLF
    "test"
    "GET / HTT...";

  char const *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "4";

  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
