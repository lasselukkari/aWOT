#include <ArduinoUnitTests.h>

#include "../src/aWOT.h"
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
  Application app;

  // Define headers on stack
  char valueBuffer[20];
  Request::HeaderNode headers[] = {
      {"X-Test", valueBuffer, sizeof(valueBuffer)},
  };

  app.get("/", &checkHeaderHandler);

  // Pass the custom headers to process
  app.process(&stream, headers, 1);

  String response = stream.response();
  assertTrue(response.endsWith("CustomValue"));
}

unittest_main()
