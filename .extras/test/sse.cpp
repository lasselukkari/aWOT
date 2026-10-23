#include <ArduinoUnitTests.h>

#include "../../src/SseChannel.h"
#include "../../src/aWOT.h"
#include "mocks/SharedClient.h"

using namespace awot;
typedef SseChannel<SharedClient, 2> Events;
static Events* opening;
static int connectedClients;

#ifdef STD_FUNCTION_MIDDLEWARE
struct StatefulSseConnectCallback {
  explicit StatefulSseConnectCallback(std::vector<int>* calls)
      : calls(calls), count(0) {}
  void operator()(Print&) { calls->push_back(++count); }
  std::vector<int>* calls;
  int count;
};
#endif

static void keepAlive(Request& req, Response& res) {
  res.set("Connection", "keep-alive");
  req.next();
}
static void openEvents(Request&, Response& res) { opening->open(res); }
static void queueHeaders(Request& req, Response& res) {
  int count = *static_cast<int*>(req.context);
  for (int i = 0; i < count; ++i) res.set("X-App", "value");
  req.next();
}
static void countConnection(Print& out) {
  ++connectedClients;
  out.print(": ready\n\n");
}
static void heartbeat(Print& out) { out.print(": heartbeat\n\n"); }

unittest(sse_open_after_keepalive_sends_close_delimited_events) {
  Events events;
  SharedClient client;
  App app;
  opening = &events;
  app.use(keepAlive);
  app.get("/events", openEvents);
  client.feed(std::string("GET /events HTTP/1.1\r\nHost: local\r\n\r\n"));
  App::ProcessResult result = app.process(&client);
  assertTrue(result.responseOpen);
  assertFalse(result.keepAlive);
  events.add(client);
  events.send("hello");
  const std::string expected =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/event-stream\r\nCache-Control: no-cache\r\n"
      "Connection: close\r\n\r\ndata: hello\n\n";
  assertEqual(expected, client.outputString());
}

unittest(sse_open_requires_capacity_for_every_response_header) {
  for (int count = SERVER_MAX_HEADERS - 2; count <= SERVER_MAX_HEADERS;
       ++count) {
    Events events;
    SharedClient client;
    App app;
    opening = &events;
    app.use(queueHeaders);
    app.get("/events", openEvents);
    client.feed("GET /events HTTP/1.1\r\nHost: local\r\n\r\n");
    App::ProcessOptions options;
    options.context = &count;
    App::ProcessResult result = app.process(&client, options);
    bool fits = count == SERVER_MAX_HEADERS - 2;
    assertEqual(fits, result.responseOpen);
    assertFalse(result.keepAlive);
    assertEqual(fits, client.connected() != 0);
    if (fits) {
      const std::string output = client.outputString();
      assertTrue(output.find("HTTP/1.1 200 OK\r\n") == 0);
      assertTrue(output.find("Content-Type: text/event-stream\r\n") !=
                 std::string::npos);
      assertTrue(output.find("Cache-Control: no-cache\r\n") !=
                 std::string::npos);
    } else {
      assertTrue(client.outputString().empty());
    }
  }
}

#ifdef STD_FUNCTION_MIDDLEWARE
unittest(sse_connect_callback_can_clear_itself_and_use_captures) {
  Events events;
  SharedClient first, second;
  const std::string captured(256, 's');
  std::string observed;
  connectedClients = 0;
  events.onConnect([&events, &observed, captured](Print& out) {
    ++connectedClients;
    events.onConnect(nullptr);
    observed = captured;
    out.print(": ready\n\n");
  });
  events.add(first);
  events.add(second);
  assertEqual(1, connectedClients);
  assertEqual(captured, observed);
  assertEqual(std::string(": ready\n\n"), first.outputString());
  assertTrue(second.outputString().empty());
}

unittest(sse_connect_callback_preserves_mutable_functor_state) {
  Events events;
  SharedClient first, second;
  std::vector<int> calls;
  events.onConnect(StatefulSseConnectCallback(&calls));
  events.add(first);
  events.add(second);
  assertTrue(calls == std::vector<int>({1, 2}));
}
#endif

unittest(sse_normalizes_multiline_payloads_and_empty_lines) {
  Events events;
  SharedClient client;
  events.add(client);
  events.send("first\r\nsecond\rthird\nfourth\n");
  events.send("");
  assertEqual(
      std::string("data: first\ndata: second\ndata: third\ndata: fourth\n"
                  "data: \n\ndata: \n\n"),
      client.outputString());
}

unittest(sse_named_events_retry_positive_short_writes) {
  Events events;
  SharedClient client;
  client.state->maxWrite = 1;
  events.add(client);
  events.send("change", "hello\nworld");
  assertEqual(std::string("event: change\ndata: hello\ndata: world\n\n"),
              client.outputString());
  assertTrue(client.connected());
}

unittest(sse_stops_failed_clients_at_prefix_payload_and_delimiter) {
  const size_t boundaries[] = {0, 3, 6, 8, 11, 12};
  for (size_t i = 0; i < sizeof(boundaries) / sizeof(boundaries[0]); ++i) {
    Events events;
    SharedClient failed, healthy;
    failed.state->failAfter = boundaries[i];
    events.add(failed);
    events.add(healthy);
    events.send("hello");
    assertFalse(failed.connected());
    assertEqual(boundaries[i], failed.state->output.size());
    assertEqual(std::string("data: hello\n\n"), healthy.outputString());
    events.send("again");
    assertEqual(boundaries[i], failed.state->output.size());
  }
}

unittest(sse_failed_named_event_does_not_continue_with_payload) {
  Events events;
  SharedClient client;
  client.state->failAfter = 9;
  events.add(client);
  events.send("change", "hello");
  assertFalse(client.connected());
  assertEqual(std::string("event: ch"), client.outputString());
}

unittest(sse_full_channel_rejects_extra_client_and_reuses_disconnected_slot) {
  Events events;
  SharedClient first, second, rejected, replacement;
  connectedClients = 0;
  events.onConnect(countConnection);
  events.add(first);
  events.add(second);
  events.add(rejected);
  assertFalse(rejected.connected());
  assertEqual(2, connectedClients);
  first.stop();
  events.send("update");
  events.add(replacement);
  assertTrue(replacement.connected());
  assertEqual(3, connectedClients);
  replacement.clearOutput();
  events.forEach(heartbeat);
  assertEqual(std::string(": heartbeat\n\n"), replacement.outputString());
}

unittest_main()
