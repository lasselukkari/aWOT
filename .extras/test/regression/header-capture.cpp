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

unittest_main()
