#include <ArduinoUnitTests.h>

#include "../../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request& req, Response& res) {
  res.set("Content-Type", "text/plain");
  res.print("/");
}

void notFound(Request& req, Response& res) {
  res.print("Custom not found page");
}

static bool finalSawCompleteHeaders;

void emptyNotFound(Request&, Response& res) {
  res.set("Content-Length", "0");
  res.end();
}

void observeCompleteHeaders(Request&, Response& res) {
  finalSawCompleteHeaders = res.headersSent();
}

unittest(root_path) {
  const char* request = "GET /notfound HTTP/1.0" CRLF CRLF;

  const char* expected =
      "HTTP/1.1 404 Not Found" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Not Found";

  MockStream stream(request);
  App app;

  app.get("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(custom_not_found) {
  const char* request = "GET /notfound HTTP/1.0" CRLF CRLF;

  const char* expected =
      "HTTP/1.1 404 Not Found" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Custom not found page";

  MockStream stream(request);
  App app;

  app.get("/", &handler);
  app.notFound(&notFound);
  app.process(&stream);

  assertEqual(expected, stream.response());
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
