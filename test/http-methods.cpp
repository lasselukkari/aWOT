#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request &req, Response &res) {
  res.set("Test", "Test");

  if(req.method() == Request::MethodType::HEAD) {
    res.end();
    return;
  }

  res.print("test");
}

const char *expected =
  "HTTP/1.1 200 OK" CRLF
  "Test: Test" CRLF
  "Content-Type: text/plain" CRLF
  "Connection: close" CRLF
  CRLF
  "test";

unittest(http_methods_get) {
  const char *request =
    "GET /test HTTP/1.0" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.get("/test", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(http_methods_post) {
  const char *request =
    "POST /test HTTP/1.0" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.post("/test", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(http_methods_put) {
  const char *request =
    "PUT /test HTTP/1.0" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.put("/test", &handler);
  app.process(&stream);
  assertEqual(expected, stream.response());
}

unittest(http_methods_patch) {
  const char *request =
    "PATCH /test HTTP/1.0" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.patch("/test", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(http_methods_delete) {
  const char *request =
    "DELETE /test HTTP/1.0" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.del("/test", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(http_methods_options) {
  const char *request =
    "OPTIONS /test HTTP/1.0" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.options("/test", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(http_methods_head) {
  const char *request =
    "HEAD /test HTTP/1.0" CRLF
    CRLF;

  const char *noBody =
    "HTTP/1.1 200 OK" CRLF
    "Test: Test" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.head("/test", &handler);
  app.process(&stream);

  assertEqual(noBody, stream.response());
}

unittest_main()
