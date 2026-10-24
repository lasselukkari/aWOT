#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

void handler(Request& req, Response& res) {
  uint8_t buffer[100] = {};

  int read = req.read(buffer, sizeof(buffer));
  if (read < 0 || (read == 0 && req.left() != 0)) {
    return res.sendStatus(400);
  }

  res.write(buffer, read);
}

void readCountHandler(Request& req, Response& res) {
  uint8_t buffer[100] = {};
  int first = req.read();
  if (first < 0) return res.sendStatus(400);
  req.push((uint8_t)first);

  int read = req.read(buffer, 100);

  res.print(read);
}

unittest(buffered_read) {
  const char* request =
      "POST / HTTP/1.0" CRLF "Content-Length: 4" CRLF CRLF "body";

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "4";

  MockStream stream(request);
  App app;

  app.post("/", &readCountHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(buffered_read_count) {
  const char* request =
      "POST / HTTP/1.0" CRLF "Content-Length: 4" CRLF CRLF "body";

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "body";

  MockStream stream(request);
  App app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(buffered_read_rejects_missing_body) {
  const char* request = "POST / HTTP/1.0" CRLF "Content-Length: 4" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 400 Bad Request" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Bad Request";

  MockStream stream(request);
  App app;
  app.post("/", handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
