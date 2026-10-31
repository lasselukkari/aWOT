#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

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
  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "CustomValue";
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

  assertEqual(expected, stream.response());
}

unittest_main()
