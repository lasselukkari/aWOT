#include <ArduinoUnitTests.h>

#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

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

  MockStream stream(request);
  Application app;

  app.get("/", &handler);
  app.process(&stream);

  String response = stream.response();

  assertTrue(response.endsWith("Value"));
}

unittest_main()
