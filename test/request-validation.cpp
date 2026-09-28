#include <ArduinoUnitTests.h>

#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

struct Observation {
  bool called;
  int minor;
  bool matched;
};

void observe(Request& req, Response& res) {
  Observation& observation = *static_cast<Observation*>(req.context);
  observation.called = true;
  observation.minor = req.minorVersion();
  res.print("ok");
}

void observeEmptyPattern(Request& req, Response& res) {
  char value[8];
  Observation& observation = *static_cast<Observation*>(req.context);
  observation.matched = req.route("id", value, sizeof(value));
  observe(req, res);
}

unittest(empty_route_pattern_has_no_named_parameters) {
  const char* mounts[] = {"/api", "/api/", "/"};
  const char* requests[] = {"GET /api HTTP/1.1" CRLF CRLF,
                            "GET /api/ HTTP/1.1" CRLF CRLF,
                            "GET / HTTP/1.1" CRLF CRLF};
  for (int i = 0; i < 3; ++i) {
    App app;
    Router router;
    router.get("", observeEmptyPattern);
    app.use(mounts[i], &router);
    MockStream stream(requests[i]);
    Observation observation = {false, -1, true};
    App::ProcessOptions options;
    options.context = &observation;
    app.process(&stream, options);
    assertTrue(observation.called);
    assertFalse(observation.matched);
  }
}

unittest(exact_http_versions) {
  const char* requests[] = {
      "GET / HTTP/1.0" CRLF CRLF,
      "GET / HTTP/1.1" CRLF CRLF,
  };
  for (int minor = 0; minor <= 1; ++minor) {
    App app;
    app.get("/", observe);
    Observation observation = {false, -1, false};
    App::ProcessOptions options;
    options.context = &observation;
    MockStream stream(requests[minor]);
    app.process(&stream, options);
    assertTrue(observation.called);
    assertEqual(minor, observation.minor);
  }
}

unittest(invalid_http_versions_never_dispatch) {
  const char* requests[] = {
      "GET / HTTP/2.0" CRLF CRLF,
      "GET / rubbish" CRLF CRLF,
      "GET / " CRLF CRLF,
      "GET / HTTP/1.10" CRLF CRLF,
      "GET / HTTP/1.1 junk" CRLF CRLF,
      "GET / http/1.1" CRLF CRLF,
  };
  for (const char* request : requests) {
    App app;
    app.get("/", observe);
    Observation observation = {false, -1, false};
    App::ProcessOptions options;
    options.context = &observation;
    MockStream stream(request);
    app.process(&stream, options);
    assertFalse(observation.called);
    assertTrue(strstr(stream.response(), "HTTP/1.1 505 ") == stream.response());
  }
}

void observeHeaders(Request& req, Response& res) {
  Observation& observation = *static_cast<Observation*>(req.context);
  observation.matched = strcmp(req.get("Accept"), "text/plain") == 0 &&
                        strcmp(req.get("Accept-Encoding"), "gzip") == 0 &&
                        strcmp(req.get("Accept-Language"), "en") == 0;
  observe(req, res);
}

unittest(header_prefixes_are_independent_of_registration_order) {
  const char* names[] = {"Accept", "Accept-Encoding", "Accept-Language"};
  const int orders[][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2},
                           {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
  for (const auto& order : orders) {
    char values[3][16];
    Request::HeaderNode headers[] = {
        {names[order[0]], values[0], sizeof(values[0]), nullptr},
        {names[order[1]], values[1], sizeof(values[1]), nullptr},
        {names[order[2]], values[2], sizeof(values[2]), nullptr},
    };
    MockStream stream("GET / HTTP/1.1" CRLF "Accept-Languages: ignored" CRLF
                      "aCcEpT-EnCoDiNg: gzip" CRLF "Accept-Language: en" CRLF
                      "Accept: text/plain" CRLF CRLF);
    App app;
    app.get("/", observeHeaders);
    Observation observation = {false, -1, false};
    App::ProcessOptions options;
    options.context = &observation;
    options.headers = headers;
    options.headerCount = 3;
    app.process(&stream, options);
    assertTrue(observation.called);
    assertTrue(observation.matched);
  }
}

const char* longHeaderA = "X-Shared-Header-Prefix-Longer-Than-Thirty-Two-A";
const char* longHeaderB = "X-Shared-Header-Prefix-Longer-Than-Thirty-Two-B";

void observeLongHeaders(Request& req, Response& res) {
  Observation& observation = *static_cast<Observation*>(req.context);
  observation.matched = strcmp(req.get(longHeaderA), "a") == 0 &&
                        strcmp(req.get(longHeaderB), "b") == 0;
  observe(req, res);
}

unittest(header_names_can_exceed_avr_pushback_capacity) {
  char a[4], b[4];
  Request::HeaderNode headers[] = {
      {longHeaderA, a, sizeof(a), nullptr},
      {longHeaderB, b, sizeof(b), nullptr},
  };
  MockStream stream(
      "GET / HTTP/1.1" CRLF
      "X-Shared-Header-Prefix-Longer-Than-Thirty-Two-B: b" CRLF
      "X-Shared-Header-Prefix-Longer-Than-Thirty-Two-A: a" CRLF CRLF);
  App app;
  app.get("/", observeLongHeaders);
  Observation observation = {false, -1, false};
  App::ProcessOptions options;
  options.context = &observation;
  options.headers = headers;
  options.headerCount = 2;
  app.process(&stream, options);
  assertTrue(observation.called);
  assertTrue(observation.matched);
}

unittest(malformed_header_names_and_framing_never_dispatch) {
  const char* requests[] = {
      "POST / HTTP/1.1" CRLF ": value" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Bad Header: value" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Content-Length : 4" CRLF CRLF "body",
      "POST / HTTP/1.1" CRLF "Content-Length: -1" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Content-Length: +1" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Content-Length:" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Content-Length: 9999999999999999999" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Content-Length: 0, 0" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Content-Length: 0" CRLF
      "Content-Length: 1" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Transfer-Encoding: chunked" CRLF CRLF,
      "POST / HTTP/1.1" CRLF "Transfer-Encoding: identity" CRLF CRLF,
  };
  for (const char* request : requests) {
    App app;
    app.post("/", observe);
    Observation observation = {false, -1, false};
    App::ProcessOptions options;
    options.context = &observation;
    MockStream stream(request);
    app.process(&stream, options);
    assertFalse(observation.called);
    assertTrue(strstr(stream.response(), "HTTP/1.1 400 ") == stream.response());
  }
}

void observeBody(Request& req, Response& res) {
  char body[5] = {};
  Observation& observation = *static_cast<Observation*>(req.context);
  observation.matched = req.left() == 4 && req.readBytes(body, 4) == 4 &&
                        strcmp(body, "body") == 0 && req.left() == 0;
  observe(req, res);
}

unittest(duplicate_content_lengths_may_agree) {
  App app;
  app.post("/", observeBody);
  Observation observation = {false, -1, false};
  App::ProcessOptions options;
  options.context = &observation;
  MockStream stream("POST / HTTP/1.1" CRLF "Content-Length: 4" CRLF
                    "content-length: \t4\t " CRLF CRLF "body");
  app.process(&stream, options);
  assertTrue(observation.called);
  assertTrue(observation.matched);
}

void observeDecodedTarget(Request& req, Response& res) {
  char value[6];
  char plus[4];
  Observation& observation = *static_cast<Observation*>(req.context);
  observation.matched =
      strcmp(req.path(), "/p?ath+") == 0 &&
      strcmp(req.query(), "first%26key=a%3Db%26c&plus=a+b") == 0 &&
      req.query("first&key", value, sizeof(value)) &&
      strcmp(value, "a=b&c") == 0 && req.query("plus", plus, sizeof(plus)) &&
      strcmp(plus, "a b") == 0;
  observe(req, res);
}

unittest(encoded_delimiters_do_not_split_query_fields) {
  App app;
  app.get("/p?ath+", observeDecodedTarget);
  Observation observation = {false, -1, false};
  App::ProcessOptions options;
  options.context = &observation;
  MockStream stream(
      "GET /p%3Fath+?first%26key=a%3Db%26c&plus=a+b HTTP/1.1" CRLF CRLF);
  app.process(&stream, options);
  assertTrue(observation.called);
  assertTrue(observation.matched);
}

unittest(invalid_percent_escapes_never_dispatch) {
  const char* requests[] = {
      "GET /bad% HTTP/1.1" CRLF CRLF,
      "GET /bad%2 HTTP/1.1" CRLF CRLF,
      "GET /bad%GG HTTP/1.1" CRLF CRLF,
      "GET /bad%00 HTTP/1.1" CRLF CRLF,
      "GET /ok?unused=% HTTP/1.1" CRLF CRLF,
      "GET /ok?unused=%00 HTTP/1.1" CRLF CRLF,
  };
  for (const char* request : requests) {
    App app;
    app.get(observe);
    Observation observation = {false, -1, false};
    App::ProcessOptions options;
    options.context = &observation;
    MockStream stream(request);
    app.process(&stream, options);
    assertFalse(observation.called);
    assertTrue(strstr(stream.response(), "HTTP/1.1 400 ") == stream.response());
  }
}

unittest_main()
