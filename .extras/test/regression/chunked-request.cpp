#include <ArduinoUnitTests.h>
#include <aWOT.h>
#include <limits.h>

#include "mocks/SharedClient.h"

static std::string request(
    const std::string& body,
    const std::string& headers = "Transfer-Encoding: chunked" CRLF,
    const char* version = "1.1") {
  return std::string("POST / HTTP/") + version + CRLF + headers + CRLF + body;
}

struct ReadObservation {
  std::string body;
  int mode = 0;
  int before = 0;
  int available = 0;
  int after = -1;
  int bytes = 0;
  int error = 0;
  bool called = false;
};

static void readReadyBody(Request& req, std::string& body, int mode) {
  if (mode == 0) {
    int ch;
    while ((ch = req.read()) >= 0) body += (char)ch;
  } else {
    uint8_t buffer[7];
    int count;
    while ((count = mode == 1 ? req.read(buffer, sizeof(buffer))
                              : req.readBytes(buffer, sizeof(buffer))) > 0)
      body.append((const char*)buffer, count);
  }
}

static void readBody(Request& req, Response& res) {
  ReadObservation& observation = *static_cast<ReadObservation*>(req.context);
  observation.called = true;
  observation.before = req.left();
  observation.available = req.available();
  readReadyBody(req, observation.body, observation.mode);
  observation.after = req.left();
  observation.bytes = req.bytesRead();
  if (observation.after == 0) {
    res.set("Connection", "keep-alive");
    res.print("ok");
  }
}

static void recordError(Request& req, Response& res, int code) {
  static_cast<ReadObservation*>(req.context)->error = code;
  res.sendStatus(code);
  res.end();
}

static App::ProcessResult process(SharedClient& client,
                                  ReadObservation& observation) {
  App app;
  app.setTimeout(0);
  app.post("/", readBody);
  app.onError(recordError);
  App::ProcessOptions options;
  options.context = &observation;
  return app.process(&client, options);
}

unittest(chunked_scalar_buffered_and_stream_reads_return_only_payload) {
  const char binary[] = {'A', 0, 'B'};
  std::string body = "3;tag=\"x;y\"" CRLF;
  body.append(binary, sizeof(binary));
  body += CRLF "0a \t;ignored=yes" CRLF "0123456789" CRLF "2" CRLF;
  body += (char)0xFF;
  body += "Z" CRLF "0;last" CRLF "X-End: yes" CRLF CRLF;
  std::string expected(binary, sizeof(binary));
  expected += "0123456789";
  expected += (char)0xFF;
  expected += 'Z';
  const std::string input = request(body);
  for (int mode = 0; mode < 3; ++mode) {
    SharedClient client;
    client.feed(input);
    ReadObservation observation;
    observation.mode = mode;

    App::ProcessResult result = process(client, observation);

    assertTrue(observation.called);
    assertEqual(-1, observation.before);
    assertEqual(3, observation.available);
    assertEqual(expected, observation.body);
    assertEqual(0, observation.after);
    assertEqual((int)input.size(), observation.bytes);
    assertEqual(0, observation.error);
    assertTrue(result.keepAlive);
    assertFalse(result.responseOpen);
  }
}

unittest(chunked_header_accepts_case_and_ows_and_dispatches_empty_bodies) {
  const char* headers[] = {"Transfer-Encoding: chunked" CRLF,
                           "tRaNsFeR-EnCoDiNg:\tChUnKeD \t" CRLF};
  for (const char* header : headers) {
    SharedClient client;
    client.feed(request("0" CRLF CRLF, header));
    ReadObservation observation;

    App::ProcessResult result = process(client, observation);

    assertTrue(observation.called);
    assertEqual(0, observation.before);
    assertEqual(0, observation.available);
    assertTrue(observation.body.empty());
    assertEqual(0, observation.after);
    assertTrue(result.keepAlive);
  }
}

unittest(chunk_sizes_can_exceed_a_sixteen_bit_counter) {
  const std::string payload(65536, 'x');
  const std::string input =
      request("10000" CRLF + payload + CRLF "0" CRLF CRLF);
  for (int mode = 0; mode < 3; ++mode) {
    SharedClient client;
    client.feed(input);
    ReadObservation observation;
    observation.mode = mode;

    App::ProcessResult result = process(client, observation);

    assertEqual(payload, observation.body);
    assertEqual(0, observation.after);
    assertEqual((int)input.size(), observation.bytes);
    assertEqual(0, observation.error);
    assertTrue(result.keepAlive);
  }
}

unittest(large_unsigned_chunk_size_with_partial_payload_is_not_a_syntax_error) {
  SharedClient client;
  client.feed(request("80000000" CRLF "x"));
  ReadObservation observation;
  App app;
  app.setTimeout(0);
  app.onError(recordError);
  app.post("/", [](Request& req, Response& res) {
    readBody(req, res);
    res.print("incomplete");
  });
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertEqual(std::string("x"), observation.body);
  assertEqual(-1, observation.after);
  assertEqual(0, observation.error);
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
}

// Like GeneratedBodyClient in buffered-read.cpp, account for large reads
// without allocating their payload. The handler observes counts and only the
// first byte.
class GeneratedChunkClient : public SharedClient {
 public:
  GeneratedChunkClient() { feed(request("FFFFFFFF" CRLF)); }
  uint32_t remaining = UINT32_MAX;
  size_t lastReadLength = 0;

  int available() override {
    if (state->position < state->input.size()) return SharedClient::available();
    return (int)std::min(remaining, (uint32_t)INT_MAX);
  }
  int read() override {
    if (state->position < state->input.size())
      return state->input[state->position++];
    if (!remaining) return -1;
    --remaining;
    return 'x';
  }
  int read(uint8_t* data, size_t length) override {
    lastReadLength = length;
    int count = (int)std::min(length, (size_t)available());
    if (count) *data = 'x';
    remaining -= (uint32_t)count;
    return count;
  }
};

unittest(
    large_chunk_available_and_bulk_counts_include_pushback_without_overflow) {
  struct Observation {
    int peek;
    int available;
    int count;
    int bytes;
    uint8_t data[2];
  } observation = {};
  GeneratedChunkClient client;
  App app;
  app.post("/", [](Request& req, Response& res) {
    Observation& o = *static_cast<Observation*>(req.context);
    o.peek = req.peek();
    o.available = req.available();
    o.count = req.read(o.data, (size_t)INT_MAX + 1);
    o.bytes = req.bytesRead();
    res.print("incomplete");
  });
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertEqual((int)'x', observation.peek);
  assertEqual(INT_MAX, observation.available);
  assertEqual(INT_MAX, observation.count);
  assertEqual(INT_MAX, observation.bytes);
  assertEqual((uint8_t)'x', observation.data[0]);
  assertEqual((uint8_t)'x', observation.data[1]);
  assertEqual((size_t)INT_MAX - 1, client.lastReadLength);
  assertFalse(result.keepAlive);
}

unittest(unsupported_or_ambiguous_transfer_encodings_never_dispatch) {
  const char* headers[] = {
      "Transfer-Encoding: identity" CRLF,
      "Transfer-Encoding: gzip" CRLF,
      "Transfer-Encoding: gzip, chunked" CRLF,
      "Transfer-Encoding: chunked, gzip" CRLF,
      "Transfer-Encoding: chunked, chunked" CRLF,
      "Transfer-Encoding: chunked;parameter=value" CRLF,
      "Transfer-Encoding:" CRLF,
      "Transfer-Encoding: chunked" CRLF "Transfer-Encoding: chunked" CRLF,
      "Transfer-Encoding: chunked" CRLF "Content-Length: 0" CRLF,
      "Content-Length: 0" CRLF "Transfer-Encoding: chunked" CRLF};
  for (const char* header : headers) {
    SharedClient client;
    client.feed(request("0" CRLF CRLF, header));
    ReadObservation observation;
    App::ProcessResult result = process(client, observation);
    assertFalse(observation.called);
    assertEqual(400, observation.error);
    assertFalse(result.keepAlive);
    assertFalse(result.responseOpen);
    assertFalse(client.connected());
  }
  SharedClient client;
  client.feed(request("0" CRLF CRLF, "Transfer-Encoding: chunked" CRLF, "1.0"));
  ReadObservation observation;
  process(client, observation);
  assertFalse(observation.called);
  assertEqual(400, observation.error);
  assertFalse(client.connected());
}

unittest(exact_payload_read_consumes_ready_final_framing_without_next_request) {
  App app;
  app.post("/", [](Request& req, Response& res) {
    ReadObservation& observation = *static_cast<ReadObservation*>(req.context);
    char data[5];
    observation.body.assign(data, req.readBytes(data, sizeof(data)));
    res.set("Connection", "keep-alive");
    res.print("first");
  });
  app.get("/", [](Request&, Response& res) { res.print("second"); });
  const std::string first = request("2" CRLF "he" CRLF "3" CRLF "llo" CRLF
                                    "0" CRLF "X-End: yes" CRLF CRLF);
  const std::string next = "GET / HTTP/1.1" CRLF CRLF;
  SharedClient client;
  client.feed(first + next);
  ReadObservation observation;
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertEqual(std::string("hello"), observation.body);
  assertEqual(first.size(), client.state->position);
  assertTrue(result.keepAlive);
  assertEqual((int)next.size(), client.available());
  assertEqual((int)'G', client.peek());
  app.process(&client);
  assertEqual(0, client.available());
  assertTrue(client.outputString().find("second") != std::string::npos);
}

unittest(chunked_peek_and_pushback_do_not_duplicate_wire_bytes_or_end_early) {
  struct Observation {
    std::vector<int> values;
    std::string body;
    int firstCount;
    int secondCount;
    int finalCount;
  } observation;
  App app;
  app.post("/", [](Request& req, Response& res) {
    Observation& o = *static_cast<Observation*>(req.context);
    o.values.push_back(req.left());
    o.values.push_back(req.available());
    o.values.push_back(req.peek());
    o.values.push_back(req.left());
    o.firstCount = req.bytesRead();
    o.values.push_back(req.peek());
    o.secondCount = req.bytesRead();
    uint8_t buffer[2];
    int count = req.read(buffer, sizeof(buffer));
    if (count > 0) o.body.assign((const char*)buffer, count);
    o.values.push_back(req.left());
    req.push('!');
    o.values.push_back(req.left());
    o.values.push_back(req.available());
    o.values.push_back(req.read());
    o.values.push_back(req.left());
    o.finalCount = req.bytesRead();
    res.set("Connection", "keep-alive");
    res.print("ok");
  });
  const std::string input =
      request("1" CRLF "a" CRLF "1" CRLF "b" CRLF "0" CRLF CRLF);
  SharedClient client;
  client.feed(input);
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertTrue(observation.values ==
             std::vector<int>({-1, 1, 'a', -1, 'a', 0, 1, 1, '!', 0}));
  assertEqual(observation.firstCount, observation.secondCount);
  assertEqual((int)input.size(), observation.finalCount);
  assertEqual(std::string("ab"), observation.body);
  assertTrue(result.keepAlive);
}

class RevealedClient : public SharedClient {
 public:
  size_t visible = 0;
  int available() override {
    size_t end = std::min(visible, state->input.size());
    return end > state->position ? (int)(end - state->position) : 0;
  }
};

unittest(chunk_metadata_and_payload_resume_at_every_packet_boundary) {
  const std::string body = "2;x=y" CRLF "ab" CRLF "1" CRLF "c" CRLF "0" CRLF
                           "Trailer: value" CRLF CRLF;
  const std::string input = request(body);
  for (size_t split = 0; split <= body.size(); ++split) {
    RevealedClient client;
    client.feed(input);
    client.visible = input.size() - body.size() + split;
    ReadObservation observation;
    App app;
    app.setTimeout(0);
    app.post("/", [](Request& req, Response& res) {
      ReadObservation& o = *static_cast<ReadObservation*>(req.context);
      readReadyBody(req, o.body, 1);
      static_cast<RevealedClient*>(req.client())->visible = (size_t)-1;
      readReadyBody(req, o.body, 1);
      o.after = req.left();
      o.bytes = req.bytesRead();
      res.set("Connection", "keep-alive");
      res.print("ok");
    });
    App::ProcessOptions options;
    options.context = &observation;

    App::ProcessResult result = app.process(&client, options);

    assertEqual(std::string("abc"), observation.body);
    assertEqual(0, observation.after);
    assertEqual((int)input.size(), observation.bytes);
    assertTrue(result.keepAlive);
  }
}

unittest(form_fields_and_percent_escapes_can_cross_chunk_boundaries) {
  const std::string encoded = "na%6De=%41+%42&x=1%262";
  std::string body;
  for (char ch : encoded) body += std::string("1" CRLF) + ch + CRLF;
  body += "0" CRLF CRLF;
  ReadObservation observation;
  SharedClient client;
  client.feed(request(body));
  App app;
  app.setTimeout(0);
  app.onError(recordError);
  app.post("/", [](Request& req, Response& res) {
    ReadObservation& o = *static_cast<ReadObservation*>(req.context);
    char name[8], value[8];
    while (req.left()) {
      if (!req.form(name, sizeof(name), value, sizeof(value))) return;
      o.body += std::string(name) + "=" + value + ";";
    }
    o.after = req.left();
    res.set("Connection", "keep-alive");
    res.print("ok");
  });
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertEqual(std::string("name=A B;x=1&2;"), observation.body);
  assertEqual(0, observation.after);
  assertEqual(0, observation.error);
  assertTrue(result.keepAlive);
}

unittest(trailers_are_discarded_without_overwriting_captured_request_headers) {
  SharedClient client;
  client.feed(
      request("1" CRLF "x" CRLF "0" CRLF "X-Test: replaced" CRLF
              "Content-Length: 999" CRLF "Transfer-Encoding: identity" CRLF
              "Connection: close" CRLF CRLF,
              "Transfer-Encoding: chunked" CRLF "X-Test: original" CRLF));
  char value[16];
  Request::Header headers[] = {{"X-Test", value}};
  ReadObservation observation;
  App app;
  app.post("/", [](Request& req, Response& res) {
    ReadObservation& o = *static_cast<ReadObservation*>(req.context);
    readReadyBody(req, o.body, 0);
    o.body += req.get("X-Test");
    o.after = req.left();
    res.set("Connection", "keep-alive");
    res.print("ok");
  });
  App::ProcessOptions options;
  options.context = &observation;
  options.headers = headers;

  App::ProcessResult result = app.process(&client, options);

  assertEqual(std::string("xoriginal"), observation.body);
  assertEqual(0, observation.after);
  assertTrue(result.keepAlive);
}

unittest(malformed_chunks_and_trailers_close_with_one_bad_request_response) {
  const std::string bodies[] = {
      "" CRLF,
      "+1" CRLF,
      "-1" CRLF,
      "0x1" CRLF,
      "Z" CRLF,
      "1\nx" CRLF "0" CRLF CRLF,
      "FFFFFFFFFFFFFFFFFFFFFFFF" CRLF,
      "100000000" CRLF,
      "1;x=\x01" CRLF "x" CRLF "0" CRLF CRLF,
      std::string("1;x=") + '\0' + CRLF "x" CRLF "0" CRLF CRLF,
      "1" CRLF "x!" CRLF "0" CRLF CRLF,
      "1" CRLF "x\n0" CRLF CRLF,
      "0" CRLF "Bad Header: value" CRLF CRLF,
      "0" CRLF ": value" CRLF CRLF,
      "0" CRLF "X: bad\x01" CRLF CRLF,
      "0" CRLF "X: bad\n" CRLF};
  for (const std::string& body : bodies) {
    SharedClient client;
    client.feed(request(body));
    ReadObservation observation;

    App::ProcessResult result = process(client, observation);

    assertTrue(observation.called);
    assertEqual(400, observation.error);
    assertFalse(result.keepAlive);
    assertFalse(result.responseOpen);
    assertFalse(client.connected());
    const std::string output = client.outputString();
    assertTrue(output.find("HTTP/1.1 400 ") == 0);
    assertTrue(output.find("HTTP/1.1", 1) == std::string::npos);
  }
}

unittest(body_syntax_error_after_response_start_closes_without_second_status) {
  SharedClient client;
  client.feed(request("1" CRLF "x!"));
  ReadObservation observation;
  App app;
  app.onError(recordError);
  app.post("/", [](Request& req, Response& res) {
    ReadObservation& o = *static_cast<ReadObservation*>(req.context);
    res.keepOpen();
    res.print("started");
    readReadyBody(req, o.body, 0);
    int first = req.read();
    int second = req.read();
    o.after = first == -1 && second == -1 ? -1 : 1;
  });
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertEqual(-1, observation.after);
  assertEqual(0, observation.error);
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
  assertFalse(client.connected());
  const std::string output = client.outputString();
  assertTrue(output.find("HTTP/1.1 200 ") == 0);
  assertTrue(output.find("HTTP/1.1", 1) == std::string::npos);
}

unittest(
    body_syntax_error_preserves_already_selected_chunked_response_framing) {
  SharedClient client;
  client.feed(request("1" CRLF "x!"));
  ReadObservation observation;
  App app;
  app.post("/", [](Request& req, Response& res) {
    res.set("Connection", "keep-alive");
    res.print("okay");
    readReadyBody(req, static_cast<ReadObservation*>(req.context)->body, 0);
  });
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  const std::string output = client.outputString();
  size_t boundary = output.find(CRLF CRLF);
  assertTrue(output.find("Transfer-Encoding: chunked") != std::string::npos);
  assertTrue(boundary != std::string::npos);
  assertEqual(std::string("4" CRLF "okay" CRLF "0" CRLF CRLF),
              output.substr(boundary + 4));
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
  assertFalse(client.connected());
}

unittest(body_error_in_middleware_stops_dispatch_and_reaches_finally) {
  SharedClient client;
  client.feed(request("Z" CRLF));
  ReadObservation observation;
  App app;
  app.onError(recordError);
  app.use([](Request& req, Response&) {
    readReadyBody(req, static_cast<ReadObservation*>(req.context)->body, 0);
    req.next();
  });
  app.post("/", [](Request& req, Response& res) {
    static_cast<ReadObservation*>(req.context)->called = true;
    res.print("route");
  });
  app.finally([](Request& req, Response&) {
    ReadObservation& o = *static_cast<ReadObservation*>(req.context);
    o.before = o.error;
  });
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertFalse(observation.called);
  assertEqual(400, observation.error);
  assertEqual(400, observation.before);
  assertTrue(client.outputString().find("HTTP/1.1 400 ") == 0);
  assertFalse(result.keepAlive);
  assertFalse(client.connected());
}

unittest(truncated_chunk_framing_never_reuses_or_opens_the_connection) {
  const std::string body = "2" CRLF "hi" CRLF "0" CRLF "X: y" CRLF CRLF;
  for (size_t length = 0; length < body.size(); ++length) {
    SharedClient client;
    client.feed(request(body.substr(0, length)));
    ReadObservation observation;
    App app;
    app.setTimeout(0);
    app.post("/", [](Request& req, Response& res) {
      ReadObservation& o = *static_cast<ReadObservation*>(req.context);
      readReadyBody(req, o.body, 2);
      o.after = req.left();
      res.set("Connection", "keep-alive");
      res.keepOpen();
      res.print("incomplete");
    });
    App::ProcessOptions options;
    options.context = &observation;

    App::ProcessResult result = app.process(&client, options);

    assertEqual(-1, observation.after);
    assertFalse(result.keepAlive);
    assertFalse(result.responseOpen);
  }
}

unittest_main()
