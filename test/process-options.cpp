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
  Request::HeaderNode header = {"X", headerValue, sizeof(headerValue), nullptr};

  for (int testCase = 0; testCase < 7; ++testCase) {
    App::ProcessOptions options;
    switch (testCase) {
      case 0:
        options.urlBuffer = url;
        options.urlBufferLength = 1;
        break;
      case 1:
        options.writeBuffer = output;
        options.writeBufferLength = CHUNK_OVERHEAD;
        break;
      case 2:
        options.headerCount = -1;
        break;
      case 3:
        options.headerCount = 1;
        break;
      default:
        options.headers = &header;
        options.headerCount = 1;
        header.name = testCase == 4 ? nullptr : "X";
        header.buffer = testCase == 5 ? nullptr : headerValue;
        header.bufferLength = testCase == 6 ? 0 : sizeof(headerValue);
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
  Request::HeaderNode ignored = {nullptr, nullptr, -1, nullptr};
  App::ProcessOptions options;
  options.urlBufferLength = -1;
  options.writeBufferLength = -1;
  options.headers = &ignored;
  options.headerCount = 0;
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
  options.urlBuffer = url + 1;
  options.urlBufferLength = 2;
  App app;
  app.get("/", recordVisit);
  visited = false;
  app.process(&client, options);
  assertTrue(visited);
  assertEqual('!', url[0]);
  assertEqual('!', url[3]);
}

unittest_main()
