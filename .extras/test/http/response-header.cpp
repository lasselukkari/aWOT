#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

void handler(Request& req, Response& res) {
  res.set("X-Test", "Value");

  const char* val = res.get("X-Test");
  if (val != NULL) {
    res.print(val);
  } else {
    res.print("NULL");
  }
}

unittest(response_header) {
  const char* request = "GET / HTTP/1.1" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 200 OK" CRLF "X-Test: Value" CRLF
      "Content-Type: text/plain" CRLF "Connection: close" CRLF CRLF "Value";

  MockStream stream(request);
  App app;

  app.get("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
