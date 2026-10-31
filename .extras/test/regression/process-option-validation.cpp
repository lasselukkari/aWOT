#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/SharedClient.h"

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
