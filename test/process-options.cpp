#include <ArduinoUnitTests.h>

#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request& req, Response& res) { res.print("test"); }

unittest(process_options_basic) {
  const char* request = "GET /test HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  App app;

  app.get("/test", &handler);

  App::ProcessOptions options;
  app.process(&stream, options);

  assertEqual(expected, stream.response());
}

unittest(process_options_custom_buffers) {
  const char* request = "GET /test HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  App app;

  app.get("/test", &handler);

  char urlBuffer[128];
  uint8_t writeBuffer[256];

  App::ProcessOptions options;
  options.urlBuffer = urlBuffer;
  options.urlBufferLength = 128;
  options.writeBuffer = writeBuffer;
  options.writeBufferLength = 256;

  app.process(&stream, options);

  assertEqual(expected, stream.response());
}

unittest_main()
