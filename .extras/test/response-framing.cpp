#include <ArduinoUnitTests.h>

#include "../../src/aWOT.h"
#include "mocks/MockStream.h"

static void keepAlive(Request& req, Response& res) {
  res.set("Connection", "keep-alive");
  req.next();
}

static void earlyDefaults(Request&, Response& res) {
  res.setDefaults();
  res.print("hello");
}

static void explicitChunked(Request&, Response& res) {
  res.set("Transfer-Encoding", "chunked");
  res.set("Content-Length", "999");
  res.print("hello");
}

static void explicitChunkedOnly(Request&, Response& res) {
  res.set("Transfer-Encoding", "chunked");
  res.print("hello");
}

static void body(Request&, Response& res) {
  res.print("hidden");
  res.write('!');
  res.printP(PSTR("hidden"));
  res.end();
}

static void headOnly(Request&, Response& res) { res.end(); }

static void bodyless(Request& req, Response& res) {
  int code = *static_cast<int*>(req.context);
  res.sendStatus(code);
  body(req, res);
}

static void interim(Request&, Response& res) {
  const int codes[] = {100, 102, 103};
  for (unsigned i = 0; i < sizeof(codes) / sizeof(codes[0]); ++i) {
    res.set("Interim", "yes");
    res.status(codes[i]);
  }
  res.set("Connection", "keep-alive");
  res.print("done");
}

static void streaming(Request&, Response& res) {
  res.set("Content-Length", "999");
  res.set("Transfer-Encoding", "chunked");
  res.keepOpen();
  res.print("hello");
}

static void headStreaming(Request&, Response& res) {
  res.keepOpen();
  res.print("hidden");
}

static void switching(Request&, Response& res) {
  res.status(101);
  res.set("Connection", "Upgrade");
  res.set("Upgrade", "websocket");
  res.beginHeaders();
  res.endHeaders();
  res.keepOpen();
}

unittest(defaults_before_status_keep_chunked_framing) {
  MockStream stream("GET / HTTP/1.1" CRLF CRLF);
  App app;
  app.use(keepAlive);
  app.get("/", earlyDefaults);
  App::ProcessResult result = app.process(&stream);
  assertEqual("HTTP/1.1 200 OK" CRLF "Connection: keep-alive" CRLF
              "Content-Type: text/plain" CRLF
              "Transfer-Encoding: chunked" CRLF CRLF "5" CRLF "hello" CRLF
              "0" CRLF CRLF,
              stream.response());
  assertTrue(result.keepAlive);
  assertFalse(result.responseOpen);
}

unittest(explicit_chunking_works_with_connection_close) {
  MockStream stream("GET / HTTP/1.1" CRLF CRLF);
  App app;
  app.get("/", explicitChunked);
  App::ProcessResult result = app.process(&stream);
  assertEqual("HTTP/1.1 200 OK" CRLF "Transfer-Encoding: chunked" CRLF
              "Content-Type: text/plain" CRLF "Connection: close" CRLF CRLF
              "5" CRLF "hello" CRLF "0" CRLF CRLF,
              stream.response());
  assertFalse(result.keepAlive);
}

unittest(request_connection_close_overrides_response_keep_alive) {
  MockStream stream("GET / HTTP/1.1" CRLF
                    "Connection: keep-alive, close" CRLF CRLF);
  App app;
  app.use(keepAlive);
  app.get("/", earlyDefaults);
  App::ProcessResult result = app.process(&stream);
  assertEqual("HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
              "Connection: close" CRLF CRLF "hello",
              stream.response());
  assertFalse(result.keepAlive);
}

unittest(http_1_0_never_uses_chunked_transfer_encoding) {
  MockStream stream("GET / HTTP/1.0" CRLF "Connection: keep-alive" CRLF CRLF);
  App app;
  app.use(keepAlive);
  app.get("/", explicitChunkedOnly);
  App::ProcessResult result = app.process(&stream);
  assertEqual("HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
              "Connection: close" CRLF CRLF "hello",
              stream.response());
  assertFalse(result.keepAlive);
}

unittest(head_suppresses_payload_and_terminal_chunk) {
  App app;
  app.use(keepAlive);
  app.head("/", body);
  app.head("/empty", headOnly);
  const char* requests[] = {"HEAD / HTTP/1.1" CRLF CRLF,
                            "HEAD /empty HTTP/1.1" CRLF CRLF};
  for (unsigned i = 0; i < sizeof(requests) / sizeof(requests[0]); ++i) {
    MockStream stream(requests[i]);
    App::ProcessResult result = app.process(&stream);
    assertEqual("HTTP/1.1 200 OK" CRLF "Connection: keep-alive" CRLF
                "Content-Type: text/plain" CRLF CRLF,
                stream.response());
    assertTrue(result.keepAlive);
  }
}

unittest(head_cannot_open_streaming_response) {
  MockStream stream("HEAD /stream HTTP/1.1" CRLF CRLF);
  App app;
  app.head("/stream", headStreaming);

  App::ProcessResult result = app.process(&stream);

  assertEqual("HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
              "Connection: close" CRLF CRLF,
              stream.response());
  assertFalse(result.responseOpen);
  assertFalse(result.keepAlive);
}

unittest(head_not_found_has_no_body) {
  MockStream stream("HEAD /missing HTTP/1.1" CRLF CRLF);
  App app;
  app.process(&stream);
  assertEqual("HTTP/1.1 404 Not Found" CRLF "Content-Type: text/plain" CRLF
              "Connection: close" CRLF CRLF,
              stream.response());
}

unittest(bodyless_statuses_suppress_all_writes) {
  int codes[] = {204, 205, 304};
  const char* expected[] = {
      "HTTP/1.1 204 No Content" CRLF "Connection: keep-alive" CRLF CRLF,
      "HTTP/1.1 205 Reset Content" CRLF "Connection: keep-alive" CRLF
      "Content-Length: 0" CRLF CRLF,
      "HTTP/1.1 304 Not Modified" CRLF "Connection: keep-alive" CRLF CRLF};
  App app;
  app.use(keepAlive);
  app.get("/", bodyless);
  for (unsigned i = 0; i < sizeof(codes) / sizeof(codes[0]); ++i) {
    MockStream stream("GET / HTTP/1.1" CRLF CRLF);
    App::ProcessOptions options;
    options.context = &codes[i];
    App::ProcessResult result = app.process(&stream, options);
    assertEqual(expected[i], stream.response());
    assertTrue(result.keepAlive);
  }
}

unittest(interim_headers_do_not_leak_to_final_response) {
  MockStream stream("GET / HTTP/1.1" CRLF CRLF);
  App app;
  app.get("/", interim);
  App::ProcessResult result = app.process(&stream);
  assertEqual("HTTP/1.1 100 Continue" CRLF "Interim: yes" CRLF CRLF
              "HTTP/1.1 102 Processing" CRLF "Interim: yes" CRLF CRLF
              "HTTP/1.1 103 Early Hints" CRLF "Interim: yes" CRLF CRLF
              "HTTP/1.1 200 OK" CRLF "Connection: keep-alive" CRLF
              "Content-Type: text/plain" CRLF
              "Transfer-Encoding: chunked" CRLF CRLF "4" CRLF "done" CRLF
              "0" CRLF CRLF,
              stream.response());
  assertTrue(result.keepAlive);
}

unittest(open_response_uses_close_delimited_framing) {
  MockStream stream("GET / HTTP/1.1" CRLF CRLF);
  App app;
  app.use(keepAlive);
  app.get("/", streaming);
  App::ProcessResult result = app.process(&stream);
  assertEqual("HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
              "Connection: close" CRLF CRLF "hello",
              stream.response());
  assertTrue(result.responseOpen);
  assertFalse(result.keepAlive);
}

unittest(protocol_switch_preserves_upgrade_headers) {
  MockStream stream("GET / HTTP/1.1" CRLF CRLF);
  App app;
  app.get("/", switching);
  App::ProcessResult result = app.process(&stream);
  assertEqual("HTTP/1.1 101 Switching Protocols" CRLF "Connection: Upgrade" CRLF
              "Upgrade: websocket" CRLF CRLF,
              stream.response());
  assertTrue(result.responseOpen);
  assertFalse(result.keepAlive);
}

unittest_main()
