#include <ArduinoUnitTests.h>

#include "../../src/aWOT.h"
#include "./mocks/MockStream.h"

void checkHeaderHandler(Request& req, Response& res) {
  char* value = req.get("X-Test");
  if (value) {
    res.print(value);
  } else {
    res.print("MISSING");
  }
}

unittest(custom_headers_stack_allocation) {
  const char* request = "GET / HTTP/1.1" CRLF "X-Test: CustomValue" CRLF CRLF;
  MockStream stream(request);
  App app;

  char valueBuffer[20];
  Request::Header headers[] = {
      {"X-Test", valueBuffer},
  };

  app.get("/", &checkHeaderHandler);

  App::ProcessOptions options;
  options.headers = headers;
  app.process(&stream, options);

  String response = stream.response();
  assertTrue(response.endsWith("CustomValue"));
}

unittest_main()
