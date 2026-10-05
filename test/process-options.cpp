#include <ArduinoUnitTests.h>

#include "../src/aWOT.h"
#include "./mocks/MockStream.h"
#include "./mocks/SharedClient.h"

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

static bool visited;

static void recordVisit(Request&, Response& res) {
  visited = true;
  res.print("ok");
}

unittest(invalid_options_close_client_without_touching_buffers) {
  App app;
  app.get("/", recordVisit);
  char url[2] = {'!', '!'};
  uint8_t output[CHUNK_OVERHEAD + 1] = {};
  char headerValue[4] = "old";
  Request::Header header = {"X", headerValue};

  for (int testCase = 0; testCase < 12; ++testCase) {
    App::ProcessOptions options;
    switch (testCase) {
      case 0:
        options.urlBuffer = {url, 1};
        break;
      case 1:
        options.writeBuffer = {output, CHUNK_OVERHEAD};
        break;
      case 2:
        options.headers = {nullptr, -1};
        break;
      case 3:
        options.headers = {nullptr, 1};
        break;
      case 8:
        options.urlBuffer = {url, -1};
        break;
      case 9:
        options.writeBuffer = {output, -1};
        break;
      case 10:
        options.urlBuffer = {url, 0};
        break;
      case 11:
        options.writeBuffer = {output, 0};
        break;
      default:
        options.headers = {&header, 1};
        header.name = testCase == 4 ? nullptr : "X";
        header.buffer = {testCase == 5 ? nullptr : headerValue,
                         testCase == 6   ? 0
                         : testCase == 7 ? -1
                                         : 4};
    }
    SharedClient client;
    client.feed("GET / HTTP/1.1" CRLF CRLF);
    visited = false;
    App::ProcessResult result = app.process(&client, options);
    assertFalse(visited);
    assertFalse(client.connected());
    assertFalse(result.keepAlive);
    assertFalse(result.responseOpen);
    assertTrue(client.state->output.empty());
    assertEqual('!', url[0]);
    assertEqual("old", headerValue);
  }
}

unittest(null_buffers_ignore_unused_lengths_and_zero_header_count) {
  SharedClient client;
  client.feed("GET / HTTP/1.1" CRLF CRLF);
  Request::Header ignored = {nullptr, nullptr, -1};
  App::ProcessOptions options;
  options.urlBuffer = {nullptr, -1};
  options.writeBuffer = {nullptr, -1};
  options.headers = {&ignored, 0};
  App app;
  app.get("/", recordVisit);
  visited = false;
  app.process(&client, options);
  assertTrue(visited);
  assertTrue(client.outputString().find("200 OK") != std::string::npos);
}

unittest(minimum_url_buffer_accepts_root) {
  SharedClient client;
  client.feed("GET / HTTP/1.1" CRLF CRLF);
  char url[4] = {'!', '!', '!', '!'};
  App::ProcessOptions options;
  options.urlBuffer = {url + 1, 2};
  App app;
  app.get("/", recordVisit);
  visited = false;
  app.process(&client, options);
  assertTrue(visited);
  assertEqual('!', url[0]);
  assertEqual('!', url[3]);
}

unittest_main()
