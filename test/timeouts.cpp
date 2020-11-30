#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request &req, Response &res) {
  req.setTimeout(0);

  char c;
  if (req.readBytes(&c, 1) == 0) {
    res.sendStatus(408);
    return;
  }

  res.set("Content-Type", "text/plain");
  res.print("/");
}

unittest(timeouts) {
  const char *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 1" CRLF
    CRLF
    "1";

  const char *okResponse =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "/";

  const char *timeoutResponse =
    "HTTP/1.1 408 Request Timeout" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "Request Timeout";

  Application app;

  app.post("/", &handler);
  int requestLength = strlen(request);

  for (int length = requestLength; length >= 0; length--) {
    MockStream stream(request, length);
    app.process(&stream);

    if (length == requestLength) {
      assertEqual(okResponse, stream.response());
    } else {
      assertEqual(timeoutResponse, stream.response());
    }
  }
}

unittest_main()
