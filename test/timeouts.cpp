#include <ArduinoUnitTests.h>

#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request& req, Response& res) {
  req.setTimeout(0);

  char c;
  if (req.readBytes(&c, 1) == 0) {
    res.sendStatus(408);
    return;
  }

  res.set("Content-Type", "text/plain");
  res.print("/");
}

unittest(timeouts) {
  const char* request =
      "POST / HTTP/1.0" CRLF "Content-Length: 1" CRLF CRLF "1";

  const char* okResponse =
      "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "/";

  const char* timeoutResponse =
      "HTTP/1.1 408 Request Timeout" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Request Timeout";

  App app;

  app.setTimeout(10);
  app.post("/", &handler);
  int requestLength = strlen(request);

  for (int length = requestLength; length >= 0; length--) {
    MockStream stream(request, length);
    app.process(&stream);

    if (length == requestLength) {
      assertEqual(okResponse, stream.response());
    } else {
      assertEqual(timeoutResponse, stream.response());
    }
  }
}

class InitiallyEmptyStream : public MockStream {
 public:
  explicit InitiallyEmptyStream(const char* request) : MockStream(request) {}

  int read() override {
    if (firstRead) {
      firstRead = false;
      return -1;
    }
    return MockStream::read();
  }

 private:
  bool firstRead = true;
};

unittest(parser_zero_timeout_accepts_ready_data) {
  App app;
  app.setTimeout(0);
  app.get("/", [](Request&, Response& res) { res.print("ok"); });
  MockStream stream("GET / HTTP/1.1" CRLF CRLF);
  app.process(&stream);
  assertTrue(strstr(stream.response(), "HTTP/1.1 200 OK") == stream.response());
}

unittest(parser_retries_empty_reads_only_with_a_timeout) {
  for (unsigned long timeout : {0UL, 10UL}) {
    App app;
    app.setTimeout(timeout);
    app.get("/", [](Request&, Response& res) { res.print("ok"); });
    InitiallyEmptyStream stream("GET / HTTP/1.1" CRLF CRLF);
    app.process(&stream);
    const char* expected = timeout ? "HTTP/1.1 200 OK" : "HTTP/1.1 408 ";
    assertTrue(strstr(stream.response(), expected) == stream.response());
  }
}

unittest_main()
