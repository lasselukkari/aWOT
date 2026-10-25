#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"
#include "mocks/SharedClient.h"

static bool missingIsNull, unregisteredIsNull, emptyIsPresent,
    whitespaceIsEmpty;

static void distinguishMissingAndEmptyHeaders(Request& req, Response& res) {
  missingIsNull = req.get("X-Missing") == nullptr;
  unregisteredIsNull = req.get("X-Unregistered") == nullptr;
  emptyIsPresent = req.get("X-Empty") && !*req.get("X-Empty");
  whitespaceIsEmpty = req.get("X-Whitespace") && !*req.get("X-Whitespace");
  res.print("checked");
}

unittest(absent_headers_are_null_and_present_empty_headers_are_strings) {
  char missing[8] = "stale";
  char empty[1] = {'!'};
  char whitespace[8] = "stale";
  Request::Header headers[] = {
      {"X-Missing", missing},
      {"X-Empty", empty},
      {"X-Whitespace", whitespace},
  };
  MockStream stream("GET / HTTP/1.0" CRLF "X-Empty:" CRLF
                    "X-Whitespace: \t  " CRLF
                    "X-Unregistered: ignored" CRLF CRLF);
  App app;
  app.get("/", distinguishMissingAndEmptyHeaders);
  missingIsNull = unregisteredIsNull = emptyIsPresent = whitespaceIsEmpty =
      false;

  app.process(&stream, headers);

  assertTrue(missingIsNull);
  assertTrue(unregisteredIsNull);
  assertTrue(emptyIsPresent);
  assertTrue(whitespaceIsEmpty);
  assertTrue(String(stream.response()).endsWith("checked"));
  assertEqual("", missing);
  assertEqual("", empty);
  assertEqual("", whitespace);
}

static void echoCapturedHeader(Request& req, Response& res) {
  const char* value = req.get("x-test");
  if (value) {
    res.print("[");
    res.print(value);
    res.print("]");
  } else {
    res.print("MISSING");
  }
}

unittest(repeated_headers_preserve_empty_values) {
  const char* requests[] = {
      "GET / HTTP/1.0" CRLF "X-Test:" CRLF "X-Test: value" CRLF CRLF,
      "GET / HTTP/1.0" CRLF "X-Test: value" CRLF "X-Test:" CRLF CRLF,
      "GET / HTTP/1.0" CRLF "X-Test:" CRLF "X-Test:" CRLF CRLF,
  };
  const char* expected[] = {"[,value]", "[value,]", "[,]"};
  char value[7];
  Request::Header headers[] = {{"X-Test", value}};
  App app;
  app.get("/", echoCapturedHeader);

  for (int i = 0; i < 3; ++i) {
    MockStream stream(requests[i]);
    app.process(&stream, headers);
    assertTrue(String(stream.response()).endsWith(expected[i]));
    assertEqual("", value);
  }
}

unittest(repeated_empty_headers_require_room_for_separator) {
  char value[1];
  Request::Header headers[] = {{"X-Test", value}};
  MockStream stream("GET / HTTP/1.0" CRLF "X-Test:" CRLF "X-Test:" CRLF CRLF);
  App app;
  app.get("/", echoCapturedHeader);

  app.process(&stream, headers);

  assertTrue(strstr(stream.response(), "HTTP/1.1 431 ") == stream.response());
  assertEqual("", value);
}

unittest(header_array_overload_preserves_client_lifecycle) {
  char value[8];
  Request::Header headers[] = {{"X-Test", value}};
  SharedClient client;
  client.feed("GET / HTTP/1.0" CRLF "X-Test: client" CRLF CRLF);
  App app;
  app.get("/", echoCapturedHeader);

  App::ProcessResult result = app.process(&client, headers);

  assertTrue(client.outputString().find("[client]") != std::string::npos);
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
  assertTrue(client.connected());
  assertEqual("", value);

  client.feed("GET / HTTP/9.9" CRLF CRLF);
  client.clearOutput();
  result = app.process(&client, headers);
  assertTrue(client.outputString().find("HTTP/1.1 505 ") == 0);
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
  assertFalse(client.connected());
  assertEqual("", value);
}

unittest(reusing_headers_clears_values_and_presence_after_success_and_error) {
  char value[8] = "stale";
  Request::Header headers[] = {{"X-Test", value}};
  const char* requests[] = {
      "GET / HTTP/1.0" CRLF "X-Test: first" CRLF CRLF,
      "GET / HTTP/1.0" CRLF CRLF,
      "GET / HTTP/1.0" CRLF "X-Test:" CRLF CRLF,
      "GET / HTTP/1.0" CRLF "X-Test: partial" CRLF "Bad Header: x" CRLF CRLF,
      "GET / HTTP/1.0" CRLF CRLF,
  };
  const char* expected[] = {"[first]", "MISSING", "[]", "Bad Request",
                            "MISSING"};
  App app;
  app.get("/", echoCapturedHeader);

  for (int i = 0; i < 5; ++i) {
    MockStream stream(requests[i]);
    app.process(&stream, headers);
    assertTrue(String(stream.response()).endsWith(expected[i]));
    assertEqual("", value);
  }
}

unittest(explicit_header_capacity_preserves_adjacent_storage) {
  char storage[7] = {'!', '!', '!', '!', '!', '!', '!'};
  Request::Header headers[] = {{"X-Test", storage + 1, 5}};
  App app;
  app.get("/", echoCapturedHeader);

  MockStream fits("GET / HTTP/1.0" CRLF "X-Test: test" CRLF CRLF);
  app.process(&fits, headers);
  assertTrue(String(fits.response()).endsWith("[test]"));
  assertEqual('!', storage[0]);
  assertEqual('!', storage[6]);
  assertEqual("", storage + 1);

  MockStream overflows("GET / HTTP/1.0" CRLF "X-Test: tests" CRLF CRLF);
  app.process(&overflows, headers);
  assertTrue(strstr(overflows.response(), "HTTP/1.1 431 ") ==
             overflows.response());
  assertEqual('!', storage[0]);
  assertEqual('!', storage[6]);
  assertEqual("", storage + 1);
}

struct FramingCapture {
  const char* name;
  const char* expected;
  int initialLeft;
  bool called;
  bool captured;
  bool bodyRead;
};

static void readCapturedFraming(Request& req, Response& res) {
  FramingCapture& observation = *static_cast<FramingCapture*>(req.context);
  observation.called = true;
  const char* value = req.get(observation.name);
  observation.captured = value && strcmp(value, observation.expected) == 0;
  char body[4];
  observation.bodyRead =
      req.left() == observation.initialLeft &&
      req.read(reinterpret_cast<uint8_t*>(body), sizeof(body)) == sizeof(body) &&
      memcmp(body, "body", sizeof(body)) == 0 && req.left() == 0;
  res.set("Connection", "keep-alive");
  res.print("ok");
}

static std::string framingRequest(const char* fields, bool chunked) {
  return std::string("POST / HTTP/1.1" CRLF) + fields + CRLF CRLF +
         (chunked ? "1" CRLF "b" CRLF "3" CRLF "ody" CRLF "0" CRLF CRLF
                  : "body");
}

unittest(framing_headers_capture_normalized_values_and_decode_bodies) {
  const struct {
    const char* fields;
    const char* name;
    const char* expected;
    bool chunked;
  } cases[] = {
      {"Content-Length: 4", "Content-Length", "4", false},
      {"cOnTeNt-LeNgTh: \t0004 \t", "content-length", "4", false},
      {"Content-Length: 4" CRLF "content-length: \t04 \t",
       "Content-Length", "4", false},
      {"Transfer-Encoding: chunked", "Transfer-Encoding", "chunked", true},
      {"tRaNsFeR-EnCoDiNg: \tChUnKeD \t", "transfer-encoding", "chunked",
       true},
  };
  App app;
  app.post("/", readCapturedFraming);
  for (const auto& test : cases) {
    char length[16], encoding[16];
    Request::Header headers[] = {{"Content-Length", length},
                                 {"Transfer-Encoding", encoding}};
    FramingCapture observation = {test.name, test.expected,
                                  test.chunked ? -1 : 4, false, false, false};
    App::ProcessOptions options;
    options.headers = headers;
    options.context = &observation;
    SharedClient client;
    client.feed(framingRequest(test.fields, test.chunked));

    App::ProcessResult result = app.process(&client, options);

    assertTrue(observation.called);
    assertTrue(observation.captured);
    assertTrue(observation.bodyRead);
    assertTrue(result.keepAlive);
    assertEqual(0, client.available());
    assertEqual("", length);
    assertEqual("", encoding);
  }
}

unittest(framing_header_capture_requires_space_for_the_complete_value) {
  const struct {
    const char* fields;
    const char* name;
    const char* expected;
    bool chunked;
  } cases[] = {
      {"Content-Length: 0004 \t", "Content-Length", "4", false},
      {"Content-Length: 4" CRLF "Content-Length: 04 \t",
       "Content-Length", "4", false},
      {"Transfer-Encoding: chunked ", "Transfer-Encoding", "chunked", true},
  };
  App app;
  app.post("/", readCapturedFraming);
  for (const auto& test : cases) {
    for (int fits = 0; fits < 2; ++fits) {
      char storage[12];
      memset(storage, '!', sizeof(storage));
      int capacity = strlen(test.expected) + fits;
      Request::Header headers[] = {{test.name, storage + 1, capacity}};
      FramingCapture observation = {test.name, test.expected,
                                    test.chunked ? -1 : 4, false, false, false};
      App::ProcessOptions options;
      options.headers = headers;
      options.context = &observation;
      SharedClient client;
      client.feed(framingRequest(test.fields, test.chunked));

      App::ProcessResult result = app.process(&client, options);

      assertEqual(fits != 0, observation.called);
      assertEqual(fits != 0, result.keepAlive);
      if (fits) {
        assertTrue(observation.captured);
        assertTrue(observation.bodyRead);
      } else {
        assertTrue(client.outputString().find("HTTP/1.1 431 ") == 0);
        assertFalse(result.responseOpen);
        assertFalse(client.connected());
      }
      assertEqual('!', storage[0]);
      assertEqual('!', storage[capacity + 1]);
      assertEqual("", storage + 1);
    }
  }
}

unittest(captured_zero_content_length_is_distinct_from_an_absent_header) {
  App app;
  app.post("/", [](Request& req, Response& res) {
    if (req.get("Transfer-Encoding") || req.left() != 0)
      return res.sendStatus(400);
    const char* length = req.get("Content-Length");
    res.print(length ? length : "missing");
  });
  const char* requests[] = {
      "POST / HTTP/1.1" CRLF "Content-Length: 0" CRLF CRLF,
      "POST / HTTP/1.1" CRLF CRLF,
  };
  char length[2], encoding[8];
  Request::Header headers[] = {{"Content-Length", length},
                               {"Transfer-Encoding", encoding}};
  for (int i = 0; i < 2; ++i) {
    MockStream stream(requests[i]);
    app.process(&stream, headers);
    assertTrue(String(stream.response()).endsWith(i == 0 ? "0" : "missing"));
  }
}

unittest(capturing_framing_headers_does_not_bypass_validation) {
  const char* requests[] = {
      "POST / HTTP/1.1" CRLF "Content-Length:" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Content-Length: +4" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Content-Length: 9999999999999999999" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Content-Length: 4, 4" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Content-Length: 4" CRLF
      "Content-Length: 5" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Transfer-Encoding:" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Transfer-Encoding: gzip" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Transfer-Encoding: chunked, gzip" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Transfer-Encoding: chunked" CRLF
      "Transfer-Encoding: chunked" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Transfer-Encoding: chunked" CRLF
      "Content-Length: 0" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Content-Length: 0" CRLF
      "Transfer-Encoding: chunked" CRLF CRLF,
      "POST / HTTP/1.0" CRLF "Transfer-Encoding: chunked" CRLF CRLF,
  };
  App app;
  app.setTimeout(0);
  app.post("/", readCapturedFraming);
  for (const char* request : requests) {
    char length[32], encoding[32];
    Request::Header headers[] = {{"Content-Length", length},
                                 {"Transfer-Encoding", encoding}};
    FramingCapture observation = {"Content-Length", "", 0, false, false,
                                  false};
    App::ProcessOptions options;
    options.headers = headers;
    options.context = &observation;
    SharedClient client;
    client.feed(request);

    App::ProcessResult result = app.process(&client, options);

    assertFalse(observation.called);
    assertTrue(client.outputString().find("HTTP/1.1 400 ") == 0);
    assertFalse(result.keepAlive);
    assertFalse(result.responseOpen);
    assertFalse(client.connected());
    assertEqual("", length);
    assertEqual("", encoding);
  }
}

unittest_main()
