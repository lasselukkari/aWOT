#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

static bool finalSawCompleteHeaders;

void emptyNotFound(Request&, Response& res) {
  res.set("Content-Length", "0");
  res.end();
}

void observeCompleteHeaders(Request&, Response& res) {
  finalSawCompleteHeaders = res.headersSent();
}

unittest(bodyless_custom_not_found_completes_headers_before_finally) {
  const char* request = "HEAD /notfound HTTP/1.1" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 404 Not Found" CRLF "Content-Length: 0" CRLF
      "Content-Type: text/plain" CRLF "Connection: close" CRLF CRLF;

  MockStream stream(request);
  App app;
  finalSawCompleteHeaders = false;
  app.notFound(emptyNotFound);
  app.finally(observeCompleteHeaders);
  app.process(&stream);

  assertEqual(expected, stream.response());
  assertTrue(finalSawCompleteHeaders);
}

unittest_main()
