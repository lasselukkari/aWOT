#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include <string>

#include "mocks/MockStream.h"
#include "mocks/SharedClient.h"

static void continueRequest(Request&, Response& res) {
  res.set("Before-Continue", "TestBefore");
  res.status(100);
  res.set("After-Continue", "TestAfter");
  res.sendStatus(204);
}

unittest(status_continue) {
  const char* request = "POST / HTTP/1.1" CRLF "Expect: 100-continue" CRLF
                        "Content-Length: 4" CRLF CRLF "test";

  const char* expected =
      "HTTP/1.1 100 Continue" CRLF "Before-Continue: TestBefore" CRLF CRLF
      "HTTP/1.1 204 No Content" CRLF "After-Continue: TestAfter" CRLF
      "Connection: close" CRLF CRLF;

  MockStream stream(request);
  App app;

  app.post("/", continueRequest);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

static void sendConfiguredStatus(Request& req, Response& res) {
  int code = *static_cast<int*>(req.context);
  res.sendStatus(code);
}

unittest(final_statuses_send_expected_reason_and_body) {
  const struct {
    int code;
    const char* statusLine;
    const char* body;
  } cases[] = {
      {200, "HTTP/1.1 200 OK", "OK"},
      {201, "HTTP/1.1 201 Created", "Created"},
      {202, "HTTP/1.1 202 Accepted", "Accepted"},
      {203, "HTTP/1.1 203 Non-Authoritative Information",
       "Non-Authoritative Information"},
      {206, "HTTP/1.1 206 Partial Content", "Partial Content"},
      {207, "HTTP/1.1 207 Multi-Status", "Multi-Status"},
      {208, "HTTP/1.1 208 Already Reported", "Already Reported"},
      {226, "HTTP/1.1 226 IM Used", "IM Used"},
      {300, "HTTP/1.1 300 Multiple Choices", "Multiple Choices"},
      {301, "HTTP/1.1 301 Moved Permanently", "Moved Permanently"},
      {302, "HTTP/1.1 302 Found", "Found"},
      {303, "HTTP/1.1 303 See Other", "See Other"},
      {305, "HTTP/1.1 305 Use Proxy", "Use Proxy"},
      {306, "HTTP/1.1 306 (Unused)", "(Unused)"},
      {307, "HTTP/1.1 307 Temporary Redirect", "Temporary Redirect"},
      {308, "HTTP/1.1 308 Permanent Redirect", "Permanent Redirect"},
      {400, "HTTP/1.1 400 Bad Request", "Bad Request"},
      {401, "HTTP/1.1 401 Unauthorized", "Unauthorized"},
      {402, "HTTP/1.1 402 Payment Required", "Payment Required"},
      {403, "HTTP/1.1 403 Forbidden", "Forbidden"},
      {404, "HTTP/1.1 404 Not Found", "Not Found"},
      {405, "HTTP/1.1 405 Method Not Allowed", "Method Not Allowed"},
      {406, "HTTP/1.1 406 Not Acceptable", "Not Acceptable"},
      {407, "HTTP/1.1 407 Proxy Authentication Required",
       "Proxy Authentication Required"},
      {408, "HTTP/1.1 408 Request Timeout", "Request Timeout"},
      {409, "HTTP/1.1 409 Conflict", "Conflict"},
      {410, "HTTP/1.1 410 Gone", "Gone"},
      {411, "HTTP/1.1 411 Length Required", "Length Required"},
      {412, "HTTP/1.1 412 Precondition Failed", "Precondition Failed"},
      {413, "HTTP/1.1 413 Payload Too Large", "Payload Too Large"},
      {414, "HTTP/1.1 414 URI Too Long", "URI Too Long"},
      {415, "HTTP/1.1 415 Unsupported Media Type", "Unsupported Media Type"},
      {416, "HTTP/1.1 416 Range Not Satisfiable", "Range Not Satisfiable"},
      {417, "HTTP/1.1 417 Expectation Failed", "Expectation Failed"},
      {421, "HTTP/1.1 421 Misdirected Request", "Misdirected Request"},
      {422, "HTTP/1.1 422 Unprocessable Entity", "Unprocessable Entity"},
      {423, "HTTP/1.1 423 Locked", "Locked"},
      {424, "HTTP/1.1 424 Failed Dependency", "Failed Dependency"},
      {425, "HTTP/1.1 425 Too Early", "Too Early"},
      {426, "HTTP/1.1 426 Upgrade Required", "Upgrade Required"},
      {428, "HTTP/1.1 428 Precondition Required", "Precondition Required"},
      {429, "HTTP/1.1 429 Too Many Requests", "Too Many Requests"},
      {431, "HTTP/1.1 431 Request Header Fields Too Large",
       "Request Header Fields Too Large"},
      {451, "HTTP/1.1 451 Unavailable For Legal Reasons",
       "Unavailable For Legal Reasons"},
      {500, "HTTP/1.1 500 Internal Server Error", "Internal Server Error"},
      {501, "HTTP/1.1 501 Not Implemented", "Not Implemented"},
      {502, "HTTP/1.1 502 Bad Gateway", "Bad Gateway"},
      {503, "HTTP/1.1 503 Service Unavailable", "Service Unavailable"},
      {504, "HTTP/1.1 504 Gateway Timeout", "Gateway Timeout"},
      {505, "HTTP/1.1 505 HTTP Version Not Supported",
       "HTTP Version Not Supported"},
      {506, "HTTP/1.1 506 Variant Also Negotiates", "Variant Also Negotiates"},
      {507, "HTTP/1.1 507 Insufficient Storage", "Insufficient Storage"},
      {508, "HTTP/1.1 508 Loop Detected", "Loop Detected"},
      {510, "HTTP/1.1 510 Not Extended", "Not Extended"},
      {511, "HTTP/1.1 511 Network Authentication Required",
       "Network Authentication Required"},
  };

  // These literals check sendStatus() formatting, including legacy phrases.
  // Application-specific headers (such as Location) are supplied by handlers.
  for (const auto& testCase : cases) {
    const char* request = "GET /status HTTP/1.1" CRLF CRLF;
    std::string expected = testCase.statusLine;
    expected +=
        CRLF "Content-Type: text/plain" CRLF "Connection: close" CRLF CRLF;
    expected += testCase.body;

    int code = testCase.code;
    App::ProcessOptions options;
    options.context = &code;
    MockStream stream(request);
    App app;
    app.get("/status", sendConfiguredStatus);

    App::ProcessResult result = app.process(&stream, options);

    assertEqual(expected, std::string(stream.response()));
    assertFalse(result.keepAlive);
    assertFalse(result.responseOpen);
  }
}

static void sendBodylessStatus(Request& req, Response& res) {
  sendConfiguredStatus(req, res);
  res.print("This body must be suppressed.");
}

unittest(bodyless_statuses_omit_reason_body_and_later_writes) {
  const struct {
    int code;
    const char* expected;
  } cases[] = {
      {204, "HTTP/1.1 204 No Content" CRLF "Connection: close" CRLF CRLF},
      {205, "HTTP/1.1 205 Reset Content" CRLF "Content-Length: 0" CRLF
            "Connection: close" CRLF CRLF},
      {304, "HTTP/1.1 304 Not Modified" CRLF "Connection: close" CRLF CRLF},
  };

  for (const auto& testCase : cases) {
    const char* request = "GET /status HTTP/1.1" CRLF CRLF;
    int code = testCase.code;
    App::ProcessOptions options;
    options.context = &code;
    MockStream stream(request);
    App app;
    app.get("/status", sendBodylessStatus);

    App::ProcessResult result = app.process(&stream, options);

    assertEqual(testCase.expected, stream.response());
    assertFalse(result.keepAlive);
    assertFalse(result.responseOpen);
  }
}

unittest(head_status_omits_reason_body) {
  const char* request = "HEAD /status HTTP/1.1" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 404 Not Found" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF;

  int code = 404;
  App::ProcessOptions options;
  options.context = &code;
  MockStream stream(request);
  App app;
  app.head("/status", sendConfiguredStatus);

  App::ProcessResult result = app.process(&stream, options);

  assertEqual(expected, stream.response());
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
}

static void sendInterimStatuses(Request&, Response& res) {
  res.set("Interim", "processing");
  res.sendStatus(102);
  res.set("Link", "</style.css>; rel=preload; as=style");
  res.sendStatus(103);
  res.set("Content-Type", "text/html");
  res.printP(PSTR("<p>ready</p>"));
}

unittest(interim_statuses_finish_with_one_final_response) {
  const char* request = "GET /page HTTP/1.1" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 102 Processing" CRLF "Interim: processing" CRLF CRLF
      "HTTP/1.1 103 Early Hints" CRLF
      "Link: </style.css>; rel=preload; as=style" CRLF CRLF
      "HTTP/1.1 200 OK" CRLF "Content-Type: text/html" CRLF
      "Connection: close" CRLF CRLF "<p>ready</p>";

  MockStream stream(request);
  App app;
  app.get("/page", sendInterimStatuses);

  App::ProcessResult result = app.process(&stream);

  assertEqual(expected, stream.response());
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
}

static void switchProtocol(Request&, Response& res) {
  res.status(101);
  res.set("Connection", "Upgrade");
  res.set("Upgrade", "awot-test");
  res.keepOpen();
  res.endHeaders();
}

unittest(switching_protocols_leaves_client_open_after_headers) {
  const char* request = "GET /upgrade HTTP/1.1" CRLF "Connection: Upgrade" CRLF
                        "Upgrade: awot-test" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 101 Switching Protocols" CRLF "Connection: Upgrade" CRLF
      "Upgrade: awot-test" CRLF CRLF;

  SharedClient client;
  client.feed(request);
  App app;
  app.get("/upgrade", switchProtocol);

  App::ProcessResult result = app.process(&client);

  assertEqual(std::string(expected), client.outputString());
  // The caller retains the client for the upgraded protocol.
  assertTrue(result.responseOpen);
  assertFalse(result.keepAlive);
  assertTrue(client.connected());
}

unittest(unknown_status_uses_numeric_reason_and_body) {
  const char* request = "GET /status HTTP/1.1" CRLF CRLF;
  const char* expected = "HTTP/1.1 599 599" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "599";

  int code = 599;
  App::ProcessOptions options;
  options.context = &code;
  MockStream stream(request);
  App app;
  app.get("/status", sendConfiguredStatus);

  App::ProcessResult result = app.process(&stream, options);

  assertEqual(expected, stream.response());
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
}

unittest_main()
