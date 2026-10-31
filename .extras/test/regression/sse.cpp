#include <ArduinoUnitTests.h>
#include <SseChannel.h>
#include <aWOT.h>

#include "mocks/SharedClient.h"

using namespace awot;
typedef SseChannel<SharedClient, 2> Events;
static Events* opening;
static int connectedClients;

#ifdef STD_FUNCTION_MIDDLEWARE
struct StatefulSseConnectCallback {
  explicit StatefulSseConnectCallback(std::vector<int>* calls)
      : calls(calls), count(0) {}
  void operator()(int) { calls->push_back(++count); }
  std::vector<int>* calls;
  int count;
};
#endif

static void openEvents(Request&, Response& res) { opening->open(res); }
static void queueHeaders(Request& req, Response& res) {
  int count = *static_cast<int*>(req.context);
  for (int i = 0; i < count; ++i) res.set("X-App", "value");
  req.next();
}
static void countConnection(int) { ++connectedClients; }
static void joinChat(int clientId) {
  opening->join(clientId, "chat");
  opening->send(clientId, "ready", "welcome");
}
static void heartbeat(Print& out) { out.print(": heartbeat\n\n"); }

unittest(sse_head_response_is_not_opened_or_retained) {
  Events events;
  SharedClient client;
  App app;
  opening = &events;
  app.head("/events", openEvents);
  client.feed("HEAD /events HTTP/1.1\r\nHost: local\r\n\r\n");

  App::ProcessResult result = app.process(&client);

  const std::string expected =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/event-stream\r\nCache-Control: no-cache\r\n"
      "Connection: close\r\n\r\n";
  assertEqual(expected, client.outputString());
  assertFalse(result.responseOpen);
  assertFalse(result.keepAlive);

  client.clearOutput();
  events.broadcast("hidden");
  assertTrue(client.outputString().empty());
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
  events.onConnect([&events, &observed, captured](int clientId) {
    ++connectedClients;
    events.onConnect(nullptr);
    observed = captured;
    events.send(clientId, "ready");
  });
  events.add(first);
  events.add(second);
  assertEqual(1, connectedClients);
  assertEqual(captured, observed);
  assertEqual(std::string("data: ready\n\n"), first.outputString());
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

unittest(sse_nested_connections_preserve_callback_and_mutable_state) {
  Events events;
  SharedClient first, second, third;
  std::vector<int> calls;
  int state = 0;
  int nestedId = -1;
  events.onConnect([&events, &second, &calls, &nestedId, state](int) mutable {
    calls.push_back(++state);
    if (state == 1) nestedId = events.add(second);
  });

  assertEqual(0, events.add(first));
  assertEqual(1, nestedId);
  assertEqual(2, events.count());
  assertTrue(calls == std::vector<int>({1, 2}));

  events.close(1);
  assertEqual(1, events.add(third));
  assertTrue(calls == std::vector<int>({1, 2, 3}));
}

unittest(sse_clearing_callback_applies_to_nested_and_later_connections) {
  Events events;
  SharedClient first, second, third;
  const std::string captured(256, 's');
  std::string observed;
  int calls = 0;
  int nestedId = -1;
  events.onConnect([&, captured](int) {
    ++calls;
    events.onConnect(nullptr);
    nestedId = events.add(second);
    observed = captured;
  });

  assertEqual(0, events.add(first));
  assertEqual(1, nestedId);
  assertEqual(captured, observed);
  events.close(1);
  assertEqual(1, events.add(third));
  assertEqual(1, calls);
}

unittest(sse_callback_replacement_survives_nested_connections) {
  Events events;
  SharedClient first, second, third;
  const std::string captured(256, 's');
  std::string observed;
  int originalCalls = 0;
  int nestedId = -1;
  std::vector<int> replacementCalls;
  events.onConnect([&, captured](int) {
    if (++originalCalls != 1) return;
    events.onConnect(StatefulSseConnectCallback(&replacementCalls));
    nestedId = events.add(second);
    observed = captured;
  });

  assertEqual(0, events.add(first));
  assertEqual(1, nestedId);
  assertEqual(captured, observed);
  assertTrue(replacementCalls == std::vector<int>({1}));

  events.close(1);
  assertEqual(1, events.add(third));
  assertEqual(1, originalCalls);
  assertTrue(replacementCalls == std::vector<int>({1, 2}));
}
#endif

unittest(sse_normalizes_multiline_payloads_and_empty_lines) {
  Events events;
  SharedClient client;
  events.add(client);
  events.broadcast("first\r\nsecond\rthird\nfourth\n");
  events.broadcast("");
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
  events.broadcast("change", "hello\nworld");
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
    events.broadcast("hello");
    assertFalse(failed.connected());
    assertEqual(boundaries[i], failed.state->output.size());
    assertEqual(std::string("data: hello\n\n"), healthy.outputString());
    events.broadcast("again");
    assertEqual(boundaries[i], failed.state->output.size());
  }
}

unittest(sse_failed_named_event_does_not_continue_with_payload) {
  Events events;
  SharedClient client;
  client.state->failAfter = 9;
  events.add(client);
  events.broadcast("change", "hello");
  assertFalse(client.connected());
  assertEqual(std::string("event: ch"), client.outputString());
}

unittest(sse_full_channel_rejects_extra_client_and_reuses_disconnected_slot) {
  Events events;
  SharedClient first, second, rejected, replacement;
  connectedClients = 0;
  events.onConnect(countConnection);
  assertEqual(0, events.add(first));
  assertEqual(1, events.add(second));
  assertEqual(-1, events.add(rejected));
  assertFalse(rejected.connected());
  assertEqual(2, connectedClients);
  first.stop();
  events.broadcast("update");
  assertEqual(0, events.add(replacement));
  assertTrue(replacement.connected());
  assertEqual(3, connectedClients);
  replacement.clearOutput();
  events.forEach(heartbeat);
  assertEqual(std::string(": heartbeat\n\n"), replacement.outputString());
}

unittest(sse_sends_plain_and_named_events_to_one_client_or_every_client) {
  Events events;
  SharedClient first, second;
  assertEqual(0, events.count());
  assertFalse(events.connected(-1));
  assertFalse(events.connected(0));
  assertFalse(events.connected(2));
  assertEqual(0, events.add(first));
  assertEqual(1, events.add(second));
  assertEqual(2, events.count());
  assertTrue(events.connected(0));
  assertTrue(events.connected(1));

  events.send(0, "first");
  events.send(1, "change", "second\r\nline\n");
  assertEqual(std::string("data: first\n\n"), first.outputString());
  assertEqual(
      std::string("event: change\ndata: second\ndata: line\ndata: \n\n"),
      second.outputString());
  first.clearOutput();
  second.clearOutput();

  events.broadcast("all");
  events.broadcast("sample", "last");
  const std::string expected = "data: all\n\nevent: sample\ndata: last\n\n";
  assertEqual(expected, first.outputString());
  assertEqual(expected, second.outputString());

  events.send(-1, "ignored");
  events.send(2, "change", "ignored");
  events.close(-1);
  events.close(2);
  assertEqual(expected, first.outputString());
  assertEqual(expected, second.outputString());
  assertEqual(2, events.count());
}

unittest(sse_room_names_are_case_sensitive_and_duplicates_preserve_capacity) {
  SseChannel<SharedClient, 2, 2> events;
  const SseChannel<SharedClient, 2, 2>& rooms = events;
  char name[] = "chat";
  char sameName[] = "chat";
  assertEqual(-1, events.defineRoom(nullptr));
  assertEqual(-1, rooms.findRoom(nullptr));
  assertEqual(-1, rooms.findRoom("chat"));
  assertEqual(0, events.defineRoom(name));
  assertEqual(0, rooms.findRoom(sameName));
  assertEqual(0, events.defineRoom(sameName));
  assertTrue(rooms.roomName(0) == name);
  assertEqual(1, events.defineRoom("Chat"));
  assertEqual(1, rooms.findRoom("Chat"));
  assertEqual(-1, rooms.findRoom("CHAT"));
  assertEqual(0, events.defineRoom(sameName));
  assertEqual(1, events.defineRoom("Chat"));
  assertEqual(-1, events.defineRoom("full"));
  assertEqual(-1, events.defineRoom(nullptr));
  assertTrue(rooms.roomName(-1) == nullptr);
  assertTrue(rooms.roomName(2) == nullptr);
}

unittest(sse_empty_room_names_are_distinct_registered_names) {
  SseChannel<SharedClient, 2, 2> events;
  assertEqual(-1, events.findRoom(""));
  assertTrue(events.roomName(0) == nullptr);
  assertEqual(0, events.defineRoom(""));
  assertEqual(0, events.findRoom(""));
  assertEqual(0, events.defineRoom(""));
  assertEqual(1, events.defineRoom("chat"));
  assertEqual(0, events.defineRoom(""));
  SharedClient client;
  assertEqual(0, events.add(client));
  events.join(0, "");
  events.to("", "hello");
  assertTrue(events.inRoom(0, ""));
  assertEqual(std::string("data: hello\n\n"), client.outputString());
}

unittest(sse_plain_and_named_room_events_use_the_same_memberships) {
  SseChannel<SharedClient, 2, 2> events;
  const SseChannel<SharedClient, 2, 2>& rooms = events;
  SharedClient first, second;
  assertEqual(0, events.defineRoom("chat"));
  assertEqual(1, events.defineRoom("other"));
  assertEqual(0, events.add(first));
  assertEqual(1, events.add(second));
  events.join(0, "chat");
  events.join(1, 1);
  assertTrue(rooms.inRoom(0, 0));
  assertTrue(rooms.inRoom(0, "chat"));
  assertFalse(rooms.inRoom(1, "chat"));

  events.to("chat", "hello");
  events.to(0, "change", "one\rtwo");
  assertEqual(
      std::string("data: hello\n\nevent: change\ndata: one\ndata: two\n\n"),
      first.outputString());
  assertTrue(second.outputString().empty());
  first.clearOutput();
  events.to(1, "other");
  events.to("other", "sample", "second");
  assertTrue(first.outputString().empty());
  assertEqual(std::string("data: other\n\nevent: sample\ndata: second\n\n"),
              second.outputString());
  second.clearOutput();

  events.join(1, "chat");
  events.leave(0, 0);
  events.to(0, "only second");
  assertFalse(rooms.inRoom(0, "chat"));
  assertTrue(rooms.inRoom(1, 0));
  assertTrue(first.outputString().empty());
  assertEqual(std::string("data: only second\n\n"), second.outputString());
  events.leave(1, "chat");
  assertFalse(rooms.inRoom(1, 0));
  assertTrue(rooms.inRoom(1, "other"));
}

unittest(
    sse_unknown_room_names_and_invalid_indices_do_not_send_or_change_rooms) {
  SseChannel<SharedClient, 2, 2> events;
  SharedClient first, second;
  assertEqual(0, events.defineRoom("chat"));
  assertEqual(0, events.add(first));
  assertEqual(1, events.add(second));
  events.join(0, 0);
  const char* names[] = {nullptr, "unknown", "Chat"};
  for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
    events.join(1, names[i]);
    events.leave(0, names[i]);
    events.to(names[i], "ignored");
    events.to(names[i], "change", "ignored");
    assertFalse(events.inRoom(0, names[i]));
    assertFalse(events.inRoom(1, names[i]));
    assertEqual(-1, events.findRoom(names[i]));
  }
  const int invalidRooms[] = {-1, 2};
  for (size_t i = 0; i < sizeof(invalidRooms) / sizeof(invalidRooms[0]); ++i) {
    events.join(1, invalidRooms[i]);
    events.leave(0, invalidRooms[i]);
    events.to(invalidRooms[i], "ignored");
    events.to(invalidRooms[i], "change", "ignored");
    assertFalse(events.inRoom(0, invalidRooms[i]));
  }
  events.join(-1, "chat");
  events.join(2, "chat");
  events.leave(-1, "chat");
  events.leave(2, "chat");
  assertFalse(events.inRoom(-1, "chat"));
  assertFalse(events.inRoom(2, "chat"));
  assertTrue(events.inRoom(0, 0));
  assertFalse(events.inRoom(1, 0));
  assertTrue(first.outputString().empty());
  assertTrue(second.outputString().empty());
  assertEqual(1, events.defineRoom("other"));
}

unittest(sse_connect_callback_can_join_and_send_to_the_assigned_slot) {
  Events events;
  SharedClient first, second;
  opening = &events;
  assertEqual(0, events.defineRoom("chat"));
  events.onConnect(joinChat);
  assertEqual(0, events.add(first));
  assertEqual(1, events.add(second));
  assertTrue(events.inRoom(0, "chat"));
  assertTrue(events.inRoom(1, "chat"));
  const std::string welcome = "event: ready\ndata: welcome\n\n";
  assertEqual(welcome, first.outputString());
  assertEqual(welcome, second.outputString());
  first.clearOutput();
  second.clearOutput();
  events.to("chat", "joined");
  assertEqual(std::string("data: joined\n\n"), first.outputString());
  assertEqual(std::string("data: joined\n\n"), second.outputString());
}

unittest(sse_disconnect_cleanup_and_slot_reuse_clear_room_memberships) {
  Events events;
  SharedClient first, second, replacement;
  assertEqual(0, events.defineRoom("chat"));
  assertEqual(0, events.add(first));
  assertEqual(1, events.add(second));
  events.join(0, "chat");
  events.join(1, "chat");
  first.stop();
  events.to("chat", "update");
  assertFalse(events.connected(0));
  assertFalse(events.inRoom(0, "chat"));
  assertEqual(1, events.count());
  assertEqual(std::string("data: update\n\n"), second.outputString());
  assertEqual(0, events.add(replacement));
  assertFalse(events.inRoom(0, "chat"));
  assertEqual(2, events.count());
  events.to("chat", "again");
  assertTrue(replacement.outputString().empty());
}

unittest(
    sse_add_reclaims_disconnected_slots_without_retaining_old_memberships) {
  Events events;
  SharedClient first, replacement;
  assertEqual(0, events.defineRoom("chat"));
  assertEqual(0, events.add(first));
  events.join(0, "chat");
  first.stop();
  assertEqual(0, events.add(replacement));
  assertTrue(events.connected(0));
  assertFalse(events.inRoom(0, "chat"));
  events.to("chat", "ignored");
  assertTrue(replacement.outputString().empty());
}

unittest(sse_close_stops_one_client_and_clears_memberships_before_reuse) {
  Events events;
  SharedClient first, second, replacement;
  assertEqual(0, events.defineRoom("chat"));
  assertEqual(0, events.add(first));
  assertEqual(1, events.add(second));
  events.join(0, "chat");
  events.join(1, "chat");
  events.close(0);
  assertFalse(first.connected());
  assertFalse(events.connected(0));
  assertFalse(events.inRoom(0, "chat"));
  assertTrue(events.connected(1));
  assertEqual(1, events.count());
  events.send(0, "ignored");
  events.close(0);
  assertTrue(first.outputString().empty());
  assertEqual(0, events.add(replacement));
  assertFalse(events.inRoom(0, "chat"));
  events.to("chat", "remaining");
  assertTrue(replacement.outputString().empty());
  assertEqual(std::string("data: remaining\n\n"), second.outputString());
}

unittest(sse_failed_room_writes_clear_memberships_and_keep_healthy_peers) {
  Events events;
  SharedClient failed, healthy, replacement;
  assertEqual(0, events.defineRoom("chat"));
  assertEqual(0, events.add(failed));
  assertEqual(1, events.add(healthy));
  events.join(0, "chat");
  events.join(1, "chat");
  failed.state->failAfter = 9;
  events.to("chat", "change", "hello");
  assertEqual(std::string("event: ch"), failed.outputString());
  assertFalse(events.connected(0));
  assertFalse(events.inRoom(0, "chat"));
  assertEqual(1, events.count());
  assertTrue(events.connected(1));
  assertTrue(events.inRoom(1, "chat"));
  assertEqual(std::string("event: change\ndata: hello\n\n"),
              healthy.outputString());
  assertEqual(0, events.add(replacement));
  assertFalse(events.inRoom(0, "chat"));
  events.to(0, "again");
  assertTrue(replacement.outputString().empty());
  events.send(0, "new client");
  assertEqual(std::string("data: new client\n\n"), replacement.outputString());
}

unittest(sse_failed_targeted_send_deactivates_only_its_recipient) {
  Events events;
  SharedClient failed, healthy;
  assertEqual(0, events.defineRoom("chat"));
  assertEqual(0, events.add(failed));
  assertEqual(1, events.add(healthy));
  events.join(0, "chat");
  failed.state->failAfter = 3;
  events.send(0, "hello");
  assertEqual(std::string("dat"), failed.outputString());
  assertFalse(events.connected(0));
  assertFalse(events.inRoom(0, "chat"));
  assertTrue(healthy.outputString().empty());
  events.send(1, "change", "healthy");
  assertEqual(std::string("event: change\ndata: healthy\n\n"),
              healthy.outputString());
}

unittest(sse_numeric_rooms_allow_the_highest_bit_before_registration) {
  SseChannel<SharedClient, 2, 32> events;
  SharedClient first, second;
  assertEqual(0, events.add(first));
  assertEqual(1, events.add(second));
  events.join(0, 31);
  events.join(1, 0);
  assertTrue(events.inRoom(0, 31));
  assertFalse(events.inRoom(0, 0));
  assertTrue(events.inRoom(1, 0));
  assertTrue(events.roomName(31) == nullptr);
  events.to(31, "high");
  assertEqual(std::string("data: high\n\n"), first.outputString());
  assertTrue(second.outputString().empty());
  events.leave(0, 31);
  assertFalse(events.inRoom(0, 31));
  events.to(31, "ignored");
  assertEqual(std::string("data: high\n\n"), first.outputString());
  events.to(0, "low");
  assertEqual(std::string("data: low\n\n"), second.outputString());
}

unittest_main()
