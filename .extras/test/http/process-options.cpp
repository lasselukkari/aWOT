#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

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

struct ProcessObservation {
  bool called;
  bool matched;
  char* url;
  char* header;
};

static void observeOptions(Request& req, Response& res) {
  ProcessObservation& observation =
      *static_cast<ProcessObservation*>(req.context);
  observation.called = true;
  observation.matched = req.path() == observation.url &&
                        req.get("X-Test") == observation.header &&
                        strcmp(req.path(), "/test") == 0 &&
                        strcmp(req.get("X-Test"), "test") == 0;
  res.print(req.get("X-Test"));
}

unittest(process_options_combines_inferred_buffers_headers_and_context) {
  const char* request = "GET /test HTTP/1.0" CRLF "X-Test: test" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  App app;

  app.get("/test", observeOptions);

  char urlBuffer[128];
  uint8_t writeBuffer[256];
  char headerBuffer[8];
  Request::Header headers[] = {{"X-Test", headerBuffer}};
  ProcessObservation observation = {false, false, urlBuffer, headerBuffer};

  App::ProcessOptions options;
  options.urlBuffer = urlBuffer;
  options.writeBuffer = writeBuffer;
  options.headers = headers;
  options.context = &observation;

  app.process(&stream, options);

  assertTrue(observation.called);
  assertTrue(observation.matched);
  assertEqual(expected, stream.response());
  assertEqual("", headerBuffer);
}

unittest_main()
