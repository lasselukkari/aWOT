#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request &req, Response &res) {
  res.status(204);
}

unittest(http_methods_get) {
  char const *request = 
  "GET /test HTTP/1.0" CRLF
  CRLF;

  char const *expected = 
  "HTTP/1.1 204 No Content" CRLF
  "Connection: close" CRLF
  CRLF;

  MockStream stream(request);
  Application app;

  app.get("/test", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(http_methods_post) {
  char const *request = 
  "POST /test HTTP/1.0" CRLF
  CRLF;

  char const *expected = 
  "HTTP/1.1 204 No Content" CRLF
  "Connection: close" CRLF
  CRLF;

  MockStream stream(request);
  Application app;

  app.post("/test", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(http_methods_put) {
  char const *request = 
  "PUT /test HTTP/1.0" CRLF
  CRLF;

  char const *expected = 
  "HTTP/1.1 204 No Content" CRLF
  "Connection: close" CRLF
  CRLF;

  MockStream stream(request);
  Application app;

  app.put("/test", &handler);
  app.process(&stream);
  assertEqual(expected, stream.response());
}

unittest(http_methods_patch) {
  char const *request = 
  "PATCH /test HTTP/1.0" CRLF
  CRLF;

  char const *expected = 
  "HTTP/1.1 204 No Content" CRLF
  "Connection: close" CRLF
  CRLF;

  MockStream stream(request);
  Application app;

  app.patch("/test", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(http_methods_delete) {
  char const *request = 
  "DELETE /test HTTP/1.0" CRLF
  CRLF;

  char const *expected = 
  "HTTP/1.1 204 No Content" CRLF
  "Connection: close" CRLF
  CRLF;

  MockStream stream(request);
  Application app;

  app.del("/test", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(http_methods_options) {
  char const *request = 
  "OPTIONS /test HTTP/1.0" CRLF
  CRLF;

  char const *expected = 
  "HTTP/1.1 204 No Content" CRLF
  "Connection: close" CRLF
  CRLF;

  MockStream stream(request);
  Application app;

  app.options("/test", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
