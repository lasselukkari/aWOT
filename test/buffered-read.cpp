#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request & req, Response & res) {
  uint8_t buffer[100] = {};

  int read = req.read(buffer , 100);

  res.write(buffer, read);
}

void readCountHandler(Request & req, Response & res) {
  uint8_t buffer[100] = {};
  req.push(req.read());

  int read = req.read(buffer , 100);

  res.print(read);
}

unittest(buffered_read) {
  const char *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 4" CRLF
    CRLF
    "body";

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "4";
    
  MockStream stream(request);
  Application app;

  app.post("/", &readCountHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}


unittest(buffered_read_count) {
  const char *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 4" CRLF
    CRLF
    "body";

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "body";
    
  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
