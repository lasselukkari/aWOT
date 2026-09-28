#include <ArduinoUnitTests.h>

#include "../src/SseChannel.h"
#include "../src/aWOT.h"
#include "mocks/SharedClient.h"

using namespace awot;
typedef SseChannel<SharedClient, 2> Events;
static Events* opening;
static int connectedClients;

static void keepAlive(Request& req, Response& res) {
  res.set("Connection", "keep-alive");
  req.next();
}
static void openEvents(Request&, Response& res) { opening->open(res); }
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
