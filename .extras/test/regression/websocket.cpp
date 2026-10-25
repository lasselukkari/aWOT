#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/SharedClient.h"

static unsigned long wsNow;
static unsigned long wsMillis() { return wsNow; }
#define millis wsMillis
#include <WsChannel.h>
#undef millis

using namespace awot;
typedef WsChannel<SharedClient, 2, 2> Channel;
static Channel* upgrading;
static bool upgradeResult;
static int connects, disconnects, messageCount, lastSocket;
static bool lastBinary;
static std::vector<uint8_t> lastData;
static int precedingHeaders;
static int precedingResponseStage;
static int callbackDepth;
static int maxCallbackDepth;
static bool nestedPayloadUnchanged;
static std::string queuedAccept;
static SharedClient* disconnectReplacement;
static bool replacementUpgradeResult;
static int closeAfterReplacement;

#ifdef STD_FUNCTION_MIDDLEWARE
struct StatefulConnectCallback {
  explicit StatefulConnectCallback(std::vector<int>* calls)
      : calls(calls), count(0) {}
  void operator()(int) { calls->push_back(++count); }
  std::vector<int>* calls;
  int count;
};

struct StatefulDisconnectCallback {
  explicit StatefulDisconnectCallback(std::vector<int>* calls)
      : calls(calls), count(0) {}
  void operator()(int) { calls->push_back(++count); }
  std::vector<int>* calls;
  int count;
};

struct StatefulMessageCallback {
  explicit StatefulMessageCallback(std::vector<int>* calls)
      : calls(calls), count(0) {}
  void operator()(WsMessage&) { calls->push_back(++count); }
  std::vector<int>* calls;
  int count;
};
#endif

static void queueHeaders(Request& req, Response& res) {
  for (int i = 0; i < precedingHeaders; ++i) res.set("X-App", "value");
  req.next();
}

static void startResponse(Request& req, Response& res) {
  res.status(200);
  if (precedingResponseStage >= 1) res.beginHeaders();
  if (precedingResponseStage >= 2) {
    res.endHeaders();
    res.print("already started");
  }
  req.next();
}

static void upgradeHandler(Request& req, Response& res) {
  upgradeResult = upgrading->upgrade(req, res);
  if (!upgradeResult) res.sendStatus(400);
}
static void inspectResponseHeaders(Request&, Response& res) {
  const char* accept = res.get("Sec-WebSocket-Accept");
  queuedAccept = accept ? accept : "";
}
static void connectHandler(int id) {
  ++connects;
  lastSocket = id;
}
static void disconnectHandler(int id) {
  ++disconnects;
  lastSocket = id;
}
static void messageHandler(WsMessage& msg) {
  ++messageCount;
  lastSocket = msg.socketId();
  lastBinary = msg.isBinary();
  lastData.clear();
  while (msg.available()) lastData.push_back((uint8_t)msg.read());
}
static void nestedPollHandler(WsMessage& msg) {
  ++messageCount;
  ++callbackDepth;
  if (callbackDepth > maxCallbackDepth) maxCallbackDepth = callbackDepth;
  const std::vector<uint8_t> before(msg.data(), msg.data() + msg.length());
  if (callbackDepth == 1) upgrading->poll();
  const std::vector<uint8_t> after(msg.data(), msg.data() + msg.length());
  nestedPayloadUnchanged = nestedPayloadUnchanged && before == after;
  lastData.insert(lastData.end(), after.begin(), after.end());
  --callbackDepth;
}

static std::string request(const std::string& key = "dGhlIHNhbXBsZSBub25jZQ==",
                           const char* method = "GET",
                           const char* version = "1.1",
                           const char* upgrade = "websocket",
                           const char* connection = "Upgrade",
                           const char* wsVersion = "13") {
  return std::string(method) + " /ws HTTP/" + version +
         "\r\nHost: localhost\r\n" + "Upgrade: " + upgrade +
         "\r\nConnection: " + connection + "\r\nSec-WebSocket-Key: " + key +
         "\r\nSec-WebSocket-Version: " + wsVersion + "\r\n\r\n";
}

static bool handshake(Channel& ws, SharedClient& client,
                      const std::string& input = request(),
                      void (*prepare)(Request&, Response&) = nullptr) {
  App app;
  upgrading = &ws;
  upgradeResult = false;
  if (prepare) app.use(prepare);
  app.use(upgradeHandler);
  app.finally(inspectResponseHeaders);
  client.feed(input);
  char upgrade[40], connection[40], key[64], version[16];
  Request::Header headers[] = {{"Upgrade", upgrade},
                               {"Connection", connection},
                               {"Sec-WebSocket-Key", key},
                               {"Sec-WebSocket-Version", version}};
  App::ProcessOptions options;
  options.headers = headers;
  App::ProcessResult result = app.process(&client, options);
  return upgradeResult && result.responseOpen;
}

static void upgradeOnDisconnect(int) {
  if (!disconnectReplacement) return;
  SharedClient* replacement = disconnectReplacement;
  disconnectReplacement = nullptr;
  replacementUpgradeResult = handshake(*upgrading, *replacement);
  if (closeAfterReplacement >= 0) upgrading->close(closeAfterReplacement);
}

static std::vector<uint8_t> frame(uint8_t opcode,
                                  const std::vector<uint8_t>& data,
                                  bool fin = true,
                                  const uint8_t* mask = nullptr) {
  std::vector<uint8_t> result;
  result.push_back((fin ? 0x80 : 0) | opcode);
  if (data.size() <= 125) {
    result.push_back(0x80 | (uint8_t)data.size());
  } else {
    result.push_back(0xFE);
    result.push_back((uint8_t)(data.size() >> 8));
    result.push_back((uint8_t)data.size());
  }
  const uint8_t defaultMask[] = {0x37, 0xFA, 0x21, 0x3D};
  if (!mask) mask = defaultMask;
  result.insert(result.end(), mask, mask + 4);
  for (size_t i = 0; i < data.size(); ++i)
    result.push_back(data[i] ^ mask[i & 3]);
  return result;
}

static bool closesWith(const std::vector<uint8_t>& input, uint16_t code) {
  Channel ws;
  SharedClient client;
  if (!handshake(ws, client)) return false;
  client.clearOutput();
  client.feed(input);
  ws.poll();
  std::vector<uint8_t> expected = {0x88, 2, (uint8_t)(code >> 8),
                                   (uint8_t)code};
  return !ws.connected(0) && !client.connected() &&
         client.state->output == expected;
}

unittest_setup() {
  wsNow = 0;
  connects = disconnects = messageCount = 0;
  lastSocket = -1;
  lastData.clear();
  queuedAccept.clear();
  disconnectReplacement = nullptr;
  replacementUpgradeResult = false;
  closeAfterReplacement = -1;
}

unittest(sha1_known_vectors_cover_empty_short_and_multiple_blocks) {
  const char* input[] = {
      "", "abc", "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
  const char* expected[] = {"da39a3ee5e6b4b0d3255bfef95601890afd80709",
                            "a9993e364706816aba3e25717850c26c9cd0d89d",
                            "84983e441c3bd26ebaae4aa1f95129e5e54670f1"};
  const char* hex = "0123456789abcdef";
  for (size_t i = 0; i < sizeof(input) / sizeof(input[0]); ++i) {
    uint8_t digest[20];
    awot::ws::sha1((const uint8_t*)input[i], strlen(input[i]), digest);
    std::string actual;
    for (size_t j = 0; j < sizeof(digest); ++j) {
      actual += hex[digest[j] >> 4];
      actual += hex[digest[j] & 15];
    }
    assertEqual(std::string(expected[i]), actual);
  }
}

unittest(handshake_accept_key_and_standard_client_transport) {
  Channel ws;
  SharedClient client;
  ws.onConnect(connectHandler);
  assertTrue(handshake(ws, client));
  assertEqual(1, connects);
  assertEqual(0, lastSocket);
  const std::string output = client.outputString();
  assertTrue(output.find("HTTP/1.1 101 Switching Protocols\r\n") == 0);
  assertTrue(
      output.find("Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n") !=
      std::string::npos);
  assertTrue(queuedAccept.empty());
  assertTrue(output.find("Transfer-Encoding") == std::string::npos);
  assertTrue(output.find("\r\n\r\n") == output.size() - 4);
}

unittest(handshake_trims_optional_whitespace_and_matches_connection_tokens) {
  Channel ws;
  SharedClient client;
  assertTrue(
      handshake(ws, client,
                request("\tdGhlIHNhbXBsZSBub25jZQ== \t", "GET", "1.1",
                        "WebSocket\t ", "keep-alive, \tUpGrAdE ", "13\t")));
  assertTrue(client.outputString().find("s3pPLMBiTxaQ9kYGzzhZRbK+xOo=") !=
             std::string::npos);
  Channel other;
  SharedClient rejected;
  assertFalse(handshake(other, rejected,
                        request("dGhlIHNhbXBsZSBub25jZQ==", "GET", "1.1",
                                "websocket", "notupgrade")));
}

unittest(handshake_rejects_invalid_key_method_and_version) {
  const char* keys[] = {"",
                        "not-base64",
                        "dGhlIHNhbXBsZSBub25jZQ=",
                        "dGhlIHNhbXBsZSBub25jZR==",
                        "dGhlIHNhbXBsZSBub25jZQ==extra",
                        "dGhlIHNhbXBsZSBub25jZQ==,dGhlIHNhbXBsZSBub25jZQ=="};
  for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    Channel ws;
    SharedClient client;
    assertFalse(handshake(ws, client, request(keys[i])));
    assertEqual(0, ws.count());
  }
  Channel ws;
  SharedClient post, oldHttp, oldWs;
  assertFalse(handshake(ws, post, request("dGhlIHNhbXBsZSBub25jZQ==", "POST")));
  assertFalse(handshake(ws, oldHttp,
                        request("dGhlIHNhbXBsZSBub25jZQ==", "GET", "1.0")));
  assertFalse(handshake(ws, oldWs,
                        request("dGhlIHNhbXBsZSBub25jZQ==", "GET", "1.1",
                                "websocket", "Upgrade", "12")));
  SharedClient withBody;
  std::string bodyRequest = request();
  bodyRequest.insert(bodyRequest.size() - 2, "Content-Length: 1\r\n");
  bodyRequest += 'x';
  assertFalse(handshake(ws, withBody, bodyRequest));
}

unittest(handshake_failure_does_not_register_socket) {
  Channel ws;
  SharedClient client;
  client.state->failAfter = 10;
  ws.onConnect(connectHandler);
  assertFalse(handshake(ws, client));
  assertEqual(0, ws.count());
  assertEqual(0, connects);
  assertFalse(client.connected());
}

unittest(generated_accept_header_does_not_consume_response_header_capacity) {
  for (precedingHeaders = SERVER_MAX_HEADERS - 2;
       precedingHeaders <= SERVER_MAX_HEADERS; ++precedingHeaders) {
    Channel ws;
    SharedClient client;
    connects = 0;
    ws.onConnect(connectHandler);
    bool fits = precedingHeaders == SERVER_MAX_HEADERS - 2;
    assertEqual(fits, handshake(ws, client, request(), queueHeaders));
    assertEqual(fits, upgradeResult);
    assertEqual(fits ? 1 : 0, ws.count());
    assertEqual(fits ? 1 : 0, connects);
    const std::string output = client.outputString();
    if (fits) {
      assertTrue(output.find("HTTP/1.1 101 Switching Protocols\r\n") == 0);
      assertTrue(output.find("Upgrade: websocket\r\n") != std::string::npos);
      assertTrue(output.find("Connection: Upgrade\r\n") != std::string::npos);
      assertTrue(output.find("Sec-WebSocket-Accept: ") != std::string::npos);
    } else {
      assertTrue(output.find("HTTP/1.1 400 Bad Request\r\n") == 0);
      assertTrue(output.find("Upgrade:") == std::string::npos);
      assertTrue(output.find("Sec-WebSocket-Accept:") == std::string::npos);
    }
  }
}

unittest(handshake_does_not_take_over_an_existing_http_response) {
  for (precedingResponseStage = 0; precedingResponseStage < 3;
       ++precedingResponseStage) {
    Channel ws;
    SharedClient client;
    connects = 0;
    ws.onConnect(connectHandler);
    assertFalse(handshake(ws, client, request(), startResponse));
    assertFalse(upgradeResult);
    assertEqual(0, ws.count());
    assertEqual(0, connects);
    assertTrue(client.outputString().find("HTTP/1.1 200 OK\r\n") == 0);
    assertTrue(client.outputString().find("Upgrade:") == std::string::npos);
  }
}

unittest(handshake_preserves_a_slot_reused_by_the_disconnect_callback) {
  Channel ws;
  SharedClient expired, nested, incoming;
  assertTrue(handshake(ws, expired));
  expired.stop();
  disconnectReplacement = &nested;
  ws.onDisconnect(upgradeOnDisconnect);

  assertTrue(handshake(ws, incoming));
  assertTrue(replacementUpgradeResult);
  assertEqual(2, ws.count());
  assertTrue(nested.connected());
  assertTrue(incoming.connected());

  nested.clearOutput();
  incoming.clearOutput();
  ws.send(0, "A");
  ws.send(1, "B");
  assertTrue(nested.state->output == std::vector<uint8_t>({0x81, 1, 'A'}));
  assertTrue(incoming.state->output == std::vector<uint8_t>({0x81, 1, 'B'}));
}

unittest(handshake_fails_if_the_disconnect_callback_fills_the_last_slot) {
  Channel ws;
  SharedClient expired, healthy, nested, incoming;
  assertTrue(handshake(ws, expired));
  assertTrue(handshake(ws, healthy));
  expired.stop();
  disconnectReplacement = &nested;
  ws.onDisconnect(upgradeOnDisconnect);

  assertFalse(handshake(ws, incoming));
  assertTrue(replacementUpgradeResult);
  assertEqual(2, ws.count());
  assertTrue(nested.connected());
  assertTrue(incoming.outputString().find("HTTP/1.1 400 Bad Request\r\n") == 0);
  assertTrue(incoming.outputString().find("101 Switching Protocols") ==
             std::string::npos);

  nested.clearOutput();
  healthy.clearOutput();
  ws.send(0, "A");
  ws.send(1, "B");
  assertTrue(nested.state->output == std::vector<uint8_t>({0x81, 1, 'A'}));
  assertTrue(healthy.state->output == std::vector<uint8_t>({0x81, 1, 'B'}));
}

unittest(handshake_uses_an_earlier_slot_freed_by_the_disconnect_callback) {
  Channel ws;
  SharedClient earlier, expired, nested, incoming;
  assertTrue(handshake(ws, earlier));
  assertTrue(handshake(ws, expired));
  expired.stop();
  disconnectReplacement = &nested;
  closeAfterReplacement = 0;
  ws.onDisconnect(upgradeOnDisconnect);

  assertTrue(handshake(ws, incoming));
  assertTrue(replacementUpgradeResult);
  assertEqual(2, ws.count());
  assertFalse(earlier.connected());
  assertTrue(nested.connected());
  assertTrue(incoming.connected());

  nested.clearOutput();
  incoming.clearOutput();
  ws.send(0, "A");
  ws.send(1, "B");
  assertTrue(incoming.state->output == std::vector<uint8_t>({0x81, 1, 'A'}));
  assertTrue(nested.state->output == std::vector<uint8_t>({0x81, 1, 'B'}));
}

unittest(nested_poll_preserves_messages_and_defers_other_sockets) {
  Channel ws;
  SharedClient first, second;
  assertTrue(handshake(ws, first));
  assertTrue(handshake(ws, second));
  callbackDepth = maxCallbackDepth = 0;
  nestedPayloadUnchanged = true;
  ws.onMessage(nestedPollHandler);
  first.feed(frame(2, {'A'}));
  second.feed(frame(2, {'B'}));
  ws.poll();
  assertEqual(2, messageCount);
  assertEqual(1, maxCallbackDepth);
  assertTrue(nestedPayloadUnchanged);
  assertTrue(lastData == std::vector<uint8_t>({'A', 'B'}));
  first.feed(frame(2, {'C'}));
  ws.poll();
  assertEqual(3, messageCount);
  assertTrue(lastData == std::vector<uint8_t>({'A', 'B', 'C'}));
}

#ifdef STD_FUNCTION_MIDDLEWARE
unittest(connect_callback_can_clear_itself_and_keep_using_captures) {
  Channel ws;
  SharedClient first, second;
  const std::string captured(256, 'c');
  std::string observed;
  ws.onConnect([&ws, &observed, captured](int) {
    ++connects;
    ws.onConnect(nullptr);
    observed = captured;
  });
  assertTrue(handshake(ws, first));
  assertTrue(handshake(ws, second));
  assertEqual(1, connects);
  assertEqual(captured, observed);
}

unittest(disconnect_callback_can_clear_itself_and_keep_using_captures) {
  Channel ws;
  SharedClient first, second;
  assertTrue(handshake(ws, first));
  assertTrue(handshake(ws, second));
  const std::string captured(256, 'd');
  std::string observed;
  ws.onDisconnect([&ws, &observed, captured](int) {
    ++disconnects;
    ws.onDisconnect(nullptr);
    observed = captured;
  });
  ws.close(0);
  ws.close(1);
  assertEqual(1, disconnects);
  assertEqual(captured, observed);
  assertEqual(0, ws.count());
}

unittest(disconnect_callback_runs_for_callback_triggered_close) {
  Channel ws;
  SharedClient first, second;
  assertTrue(handshake(ws, first));
  assertTrue(handshake(ws, second));
  std::vector<int> calls;
  ws.onDisconnect([&ws, &calls](int id) {
    calls.push_back(id);
    if (id == 0) ws.close(1);
  });

  ws.close(0);

  assertTrue(calls == std::vector<int>({0, 1}));
  assertEqual(0, ws.count());
}

unittest(disconnect_callback_runs_for_callback_broadcast_failure) {
  Channel ws;
  SharedClient first, second;
  assertTrue(handshake(ws, first));
  assertTrue(handshake(ws, second));
  std::vector<int> calls;
  ws.onDisconnect([&ws, &calls](int id) {
    calls.push_back(id);
    if (id == 0) ws.broadcast("disconnected");
  });
  second.state->failAfter = second.state->output.size();

  ws.close(0);

  assertTrue(calls == std::vector<int>({0, 1}));
  assertEqual(0, ws.count());
  assertFalse(second.connected());
}

unittest(message_callback_can_clear_or_replace_itself_and_use_captures) {
  for (int replace = 0; replace < 2; ++replace) {
    Channel ws;
    SharedClient client;
    assertTrue(handshake(ws, client));
    const std::string captured(256, 'm');
    std::string observed;
    messageCount = 0;
    ws.onMessage([&ws, &observed, captured, replace](WsMessage&) {
      ++messageCount;
      if (replace)
        ws.onMessage(messageHandler);
      else
        ws.onMessage(nullptr);
      observed = captured;
    });
    client.feed(frame(2, {'A'}));
    ws.poll();
    assertEqual(1, messageCount);
    assertEqual(captured, observed);
    client.feed(frame(2, {'B'}));
    ws.poll();
    assertEqual(replace ? 2 : 1, messageCount);
    if (replace) assertTrue(lastData == std::vector<uint8_t>({'B'}));
  }
}

unittest(callbacks_preserve_mutable_functor_state) {
  Channel ws;
  SharedClient first, second;
  std::vector<int> connectCalls, disconnectCalls, messageCalls;
  ws.onConnect(StatefulConnectCallback(&connectCalls));
  ws.onDisconnect(StatefulDisconnectCallback(&disconnectCalls));
  ws.onMessage(StatefulMessageCallback(&messageCalls));
  assertTrue(handshake(ws, first));
  assertTrue(handshake(ws, second));
  first.feed(frame(2, {'A'}));
  second.feed(frame(2, {'B'}));
  ws.poll();
  ws.close(0);
  ws.close(1);
  assertTrue(connectCalls == std::vector<int>({1, 2}));
  assertTrue(messageCalls == std::vector<int>({1, 2}));
  assertTrue(disconnectCalls == std::vector<int>({1, 2}));
}
#endif

unittest(masked_frames_can_arrive_one_byte_at_a_time) {
  Channel ws;
  SharedClient client;
  assertTrue(handshake(ws, client));
  ws.onMessage(messageHandler);
  const std::vector<uint8_t> text = {'H', 'e', 'l', 'l', 'o'};
  std::vector<uint8_t> input = frame(1, text);
  for (size_t i = 0; i < input.size(); ++i) {
    client.feed(std::vector<uint8_t>(1, input[i]));
    ws.poll();
    assertEqual(i + 1 == input.size() ? 1 : 0, messageCount);
  }
  assertTrue(lastData == text);
  assertFalse(lastBinary);
  assertEqual(0, lastSocket);
}

unittest(extended_binary_payload_at_capacity_and_empty_frames) {
  Channel ws;
  SharedClient client;
  assertTrue(handshake(ws, client));
  ws.onMessage(messageHandler);
  std::vector<uint8_t> payload(WS_RX_BUFFER_SIZE, 0xFF);
  client.feed(frame(2, payload));
  ws.poll();
  assertEqual(1, messageCount);
  assertTrue(lastBinary);
  assertTrue(lastData == payload);
  client.feed(frame(1, std::vector<uint8_t>()));
  ws.poll();
  assertEqual(2, messageCount);
  assertTrue(lastData.empty());
}

unittest(fragmented_text_and_binary_dispatch_once_with_per_frame_masks) {
  const uint8_t secondMask[] = {1, 2, 3, 4};
  const uint8_t finalMask[] = {0xA0, 0xB1, 0xC2, 0xD3};
  for (uint8_t opcode : {1, 2}) {
    Channel ws;
    SharedClient client;
    assertTrue(handshake(ws, client));
    ws.onMessage(messageHandler);
    messageCount = 0;
    std::vector<uint8_t> expected = {
        'A', (uint8_t)(opcode == 1 ? 'B' : 0xFF), 'C', 'D', 'E', 'F'};
    client.feed(frame(opcode, {expected[0], expected[1], expected[2]}, false));
    ws.poll();
    assertTrue(ws.connected(0));
    assertEqual(0, messageCount);
    client.feed(frame(0, {}, false));
    ws.poll();
    assertEqual(0, messageCount);
    client.feed(frame(0, {'D'}, false, secondMask));
    ws.poll();
    assertEqual(0, messageCount);
    client.feed(frame(0, {'E', 'F'}, true, finalMask));
    ws.poll();
    assertEqual(1, messageCount);
    assertEqual(opcode == 2, lastBinary);
    assertTrue(lastData == expected);
    client.feed(frame(opcode, {'G'}));
    ws.poll();
    assertEqual(2, messageCount);
    assertTrue(lastData == std::vector<uint8_t>({'G'}));
  }
}

unittest(empty_fragments_still_form_one_text_or_binary_message) {
  for (uint8_t opcode : {1, 2}) {
    Channel ws;
    SharedClient client;
    assertTrue(handshake(ws, client));
    ws.onMessage(messageHandler);
    messageCount = 0;
    client.feed(frame(opcode, {}, false));
    ws.poll();
    assertTrue(ws.connected(0));
    assertEqual(0, messageCount);
    client.feed(frame(0, {}, false));
    ws.poll();
    assertEqual(0, messageCount);
    client.feed(frame(0, {}));
    ws.poll();
    assertEqual(1, messageCount);
    assertEqual(opcode == 2, lastBinary);
    assertTrue(lastData.empty());
  }
}

unittest(fragmented_utf8_frames_can_arrive_one_byte_at_a_time) {
  Channel ws;
  SharedClient client;
  assertTrue(handshake(ws, client));
  ws.onMessage(messageHandler);
  std::vector<uint8_t> input = frame(1, {'A', 0xF0, 0x9F}, false);
  std::vector<uint8_t> middle = frame(0, {0x98}, false);
  std::vector<uint8_t> final = frame(0, {0x80, 'B'});
  input.insert(input.end(), middle.begin(), middle.end());
  input.insert(input.end(), final.begin(), final.end());
  for (size_t i = 0; i < input.size(); ++i) {
    client.feed(std::vector<uint8_t>(1, input[i]));
    ws.poll();
    assertTrue(ws.connected(0));
    assertEqual(i + 1 == input.size() ? 1 : 0, messageCount);
  }
  assertFalse(lastBinary);
  assertTrue(lastData ==
             std::vector<uint8_t>({'A', 0xF0, 0x9F, 0x98, 0x80, 'B'}));
}

unittest(fragmented_invalid_utf8_is_rejected_when_the_message_finishes) {
  const std::vector<uint8_t> prefixes[] = {{0xE2, 0x82}, {0xFF}, {0xE2}};
  const std::vector<uint8_t> endings[] = {{}, {}, {'x'}};
  for (size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    Channel ws;
    SharedClient client;
    assertTrue(handshake(ws, client));
    client.clearOutput();
    ws.onMessage(messageHandler);
    client.feed(frame(1, prefixes[i], false));
    ws.poll();
    assertTrue(ws.connected(0));
    assertTrue(client.state->output.empty());
    assertEqual(0, messageCount);
    client.feed(frame(0, endings[i]));
    ws.poll();
    assertFalse(ws.connected(0));
    assertEqual(0, messageCount);
    assertTrue(client.state->output ==
               std::vector<uint8_t>({0x88, 2, 3, 0xEF}));
  }
}

unittest(interleaved_ping_and_pong_preserve_the_fragmented_message) {
  Channel ws;
  SharedClient client;
  assertTrue(handshake(ws, client));
  client.clearOutput();
  ws.onMessage(messageHandler);
  client.feed(frame(1, {'h', 'e'}, false));
  ws.poll();
  client.feed(frame(9, {'p', 0xFF}));
  ws.poll();
  assertTrue(client.state->output ==
             std::vector<uint8_t>({0x8A, 2, 'p', 0xFF}));
  client.clearOutput();
  client.feed(frame(10, {'q', 0xFF}));
  ws.poll();
  assertTrue(client.state->output.empty());
  assertEqual(0, messageCount);
  client.feed(frame(0, {'l', 'l', 'o'}));
  ws.poll();
  assertEqual(1, messageCount);
  assertFalse(lastBinary);
  assertTrue(lastData == std::vector<uint8_t>({'h', 'e', 'l', 'l', 'o'}));
}

unittest(interleaved_close_discards_the_incomplete_message) {
  Channel ws;
  SharedClient client;
  assertTrue(handshake(ws, client));
  client.clearOutput();
  ws.onMessage(messageHandler);
  ws.onDisconnect(disconnectHandler);
  client.feed(frame(1, {'h', 'i'}, false));
  ws.poll();
  client.feed(frame(8, {3, 0xE8, 'b', 'y', 'e'}));
  ws.poll();
  assertEqual(0, messageCount);
  assertEqual(1, disconnects);
  assertFalse(ws.connected(0));
  assertTrue(client.state->output ==
             std::vector<uint8_t>({0x88, 5, 3, 0xE8, 'b', 'y', 'e'}));
}

unittest(full_fragmented_message_allows_a_maximum_ping_and_empty_final_frame) {
  Channel ws;
  SharedClient client;
  assertTrue(handshake(ws, client));
  client.clearOutput();
  ws.onMessage(messageHandler);
  const std::vector<uint8_t> payload(WS_RX_BUFFER_SIZE, 0xFF);
  client.feed(frame(2, payload, false));
  ws.poll();
  assertTrue(ws.connected(0));
  assertEqual(0, messageCount);
  std::vector<uint8_t> ping(125);
  for (size_t i = 0; i < ping.size(); ++i) ping[i] = (uint8_t)i;
  std::vector<uint8_t> input = frame(9, ping);
  for (size_t i = 0; i < input.size(); ++i) {
    client.feed(std::vector<uint8_t>(1, input[i]));
    ws.poll();
    assertTrue(ws.connected(0));
  }
  std::vector<uint8_t> pong = {0x8A, 125};
  pong.insert(pong.end(), ping.begin(), ping.end());
  assertTrue(client.state->output == pong);
  assertEqual(0, messageCount);
  client.feed(frame(0, {}));
  ws.poll();
  assertEqual(1, messageCount);
  assertTrue(lastBinary);
  assertTrue(lastData == payload);
}

unittest(fragmented_messages_reject_aggregate_overflow_and_invalid_sequences) {
  for (int fin = 0; fin < 2; ++fin) {
    std::vector<uint8_t> input =
        frame(2, std::vector<uint8_t>(WS_RX_BUFFER_SIZE - 1), false);
    std::vector<uint8_t> continuation = frame(0, {'a', 'b'}, fin != 0);
    input.insert(input.end(), continuation.begin(), continuation.end());
    assertTrue(closesWith(input, 1009));
    assertTrue(closesWith(frame(0, {}, fin != 0), 1002));
    for (uint8_t opcode : {1, 2}) {
      input = frame(1, {'a'}, false);
      std::vector<uint8_t> next = frame(opcode, {'b'}, fin != 0);
      input.insert(input.end(), next.begin(), next.end());
      assertTrue(closesWith(input, 1002));
    }
  }
}

unittest(disconnect_and_slot_reuse_clear_partial_message_and_control_state) {
  for (int localClose = 0; localClose < 2; ++localClose) {
    Channel ws;
    SharedClient first, replacement;
    assertTrue(handshake(ws, first));
    ws.onMessage(messageHandler);
    messageCount = 0;
    first.feed(frame(1, {'o', 'l', 'd'}, false));
    ws.poll();
    std::vector<uint8_t> ping = frame(9, {'x', 'y'});
    first.feed(std::vector<uint8_t>(ping.begin(), ping.end() - 1));
    ws.poll();
    if (localClose) {
      ws.close(0);
    } else {
      first.stop();
      ws.poll();
    }
    assertFalse(ws.connected(0));
    assertEqual(0, messageCount);
    assertTrue(handshake(ws, replacement));
    replacement.feed(frame(2, {0xFF}, false));
    ws.poll();
    assertTrue(ws.connected(0));
    assertEqual(0, messageCount);
    replacement.feed(frame(0, {0}));
    ws.poll();
    assertEqual(1, messageCount);
    assertEqual(0, lastSocket);
    assertTrue(lastBinary);
    assertTrue(lastData == std::vector<uint8_t>({0xFF, 0}));
  }
}

unittest(two_clients_keep_partial_control_frames_and_messages_independent) {
  Channel ws;
  SharedClient first, second;
  assertTrue(handshake(ws, first));
  assertTrue(handshake(ws, second));
  first.clearOutput();
  second.clearOutput();
  ws.onMessage(messageHandler);
  first.feed(frame(1, {'A'}, false));
  second.feed(frame(1, {'B'}, false));
  ws.poll();
  std::vector<uint8_t> firstPing = frame(9, {'x', 'y'});
  std::vector<uint8_t> secondPing = frame(9, {'z', 'w'});
  first.feed(std::vector<uint8_t>(firstPing.begin(), firstPing.end() - 1));
  second.feed(std::vector<uint8_t>(secondPing.begin(), secondPing.end() - 1));
  ws.poll();
  assertTrue(first.state->output.empty());
  assertTrue(second.state->output.empty());
  assertEqual(0, messageCount);
  first.feed(std::vector<uint8_t>(1, firstPing.back()));
  ws.poll();
  assertTrue(first.state->output == std::vector<uint8_t>({0x8A, 2, 'x', 'y'}));
  assertTrue(second.state->output.empty());
  second.feed(std::vector<uint8_t>(1, secondPing.back()));
  ws.poll();
  assertTrue(second.state->output == std::vector<uint8_t>({0x8A, 2, 'z', 'w'}));
  first.feed(frame(0, {'a'}));
  ws.poll();
  assertEqual(1, messageCount);
  assertEqual(0, lastSocket);
  assertTrue(lastData == std::vector<uint8_t>({'A', 'a'}));
  second.feed(frame(0, {'b'}));
  ws.poll();
  assertEqual(2, messageCount);
  assertEqual(1, lastSocket);
  assertTrue(lastData == std::vector<uint8_t>({'B', 'b'}));
}

unittest(
    text_utf8_validation_rejects_overlong_surrogate_and_truncated_sequences) {
  const std::vector<uint8_t> invalid[] = {{0xFF},
                                          {0x80},
                                          {0xC0, 0x80},
                                          {0xE0, 0x80, 0x80},
                                          {0xED, 0xA0, 0x80},
                                          {0xF0, 0x80, 0x80, 0x80},
                                          {0xF4, 0x90, 0x80, 0x80},
                                          {0xF5, 0x80, 0x80, 0x80},
                                          {0xE2, 0x82}};
  for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
    assertTrue(closesWith(frame(1, invalid[i]), 1007));
  }
  Channel ws;
  SharedClient client;
  assertTrue(handshake(ws, client));
  ws.onMessage(messageHandler);
  std::vector<uint8_t> valid = {'A',  0xC2, 0xA2, 0xE2, 0x82,
                                0xAC, 0xF4, 0x8F, 0xBF, 0xBF};
  client.feed(frame(1, valid));
  ws.poll();
  assertEqual(1, messageCount);
  assertTrue(lastData == valid);
}

unittest(invalid_frame_headers_and_oversize_payloads_close) {
  assertTrue(closesWith({0x81, 0}, 1002));
  assertTrue(closesWith({0xC1, 0x80}, 1002));
  assertTrue(closesWith({0x83, 0x80}, 1002));
  assertTrue(closesWith({0x89, 0xFE}, 1002));
  assertTrue(closesWith({0x09, 0x80}, 1002));
  assertTrue(closesWith({0x80, 0x80}, 1002));
  assertTrue(closesWith({0x81, 0xFE, 0, 1, 0, 0, 0, 0}, 1002));
  assertTrue(
      closesWith({0x81, 0xFF, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1002));
  assertTrue(
      closesWith(frame(2, std::vector<uint8_t>(WS_RX_BUFFER_SIZE + 1)), 1009));
}

unittest(close_validates_status_and_utf8_reason) {
  assertTrue(closesWith(frame(8, {1}), 1002));
  const uint16_t invalid[] = {999, 1004, 1005, 1006, 1015, 2000, 5000};
  for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
    assertTrue(closesWith(
        frame(8, {(uint8_t)(invalid[i] >> 8), (uint8_t)invalid[i]}), 1002));
  }
  assertTrue(closesWith(frame(8, {3, 0xE8, 0xFF}), 1007));
  Channel ws;
  SharedClient client;
  assertTrue(handshake(ws, client));
  client.clearOutput();
  ws.onDisconnect(disconnectHandler);
  client.feed(frame(8, {3, 0xE8, 'b', 'y', 'e'}));
  ws.poll();
  std::vector<uint8_t> expected = {0x88, 5, 3, 0xE8, 'b', 'y', 'e'};
  assertTrue(client.state->output == expected);
  assertEqual(1, disconnects);
  assertFalse(ws.connected(0));
}

unittest(empty_close_is_echoed_without_inventing_a_status) {
  Channel ws;
  SharedClient client;
  assertTrue(handshake(ws, client));
  client.clearOutput();
  client.feed(frame(8, {}));
  ws.poll();
  std::vector<uint8_t> expected = {0x88, 0};
  assertTrue(client.state->output == expected);
  assertFalse(ws.connected(0));
}

unittest(short_writes_retry_headers_and_payloads) {
  Channel ws;
  SharedClient client;
  client.state->maxWrite = 1;
  assertTrue(handshake(ws, client));
  client.clearOutput();
  ws.send(0, "hello");
  std::vector<uint8_t> expected = {0x81, 5, 'h', 'e', 'l', 'l', 'o'};
  assertTrue(client.state->output == expected);
  assertTrue(ws.connected(0));
  client.clearOutput();
  client.feed(frame(9, {'a', 'b'}));
  ws.poll();
  expected = {0x8A, 2, 'a', 'b'};
  assertTrue(client.state->output == expected);
}

unittest(failed_pong_headers_and_payloads_deactivate_socket) {
  const size_t boundaries[] = {0, 1, 2, 3};
  for (size_t i = 0; i < sizeof(boundaries) / sizeof(boundaries[0]); ++i) {
    Channel ws;
    SharedClient client;
    assertTrue(handshake(ws, client));
    client.clearOutput();
    client.state->failAfter = boundaries[i];
    ws.onDisconnect(disconnectHandler);
    client.feed(frame(9, {'a', 'b'}));
    ws.poll();
    assertFalse(ws.connected(0));
    assertFalse(client.connected());
    assertEqual(boundaries[i], client.state->output.size());
    size_t before = client.state->output.size();
    ws.send(0, "later");
    assertEqual(before, client.state->output.size());
  }
  assertEqual(4, disconnects);
}

unittest(failed_data_write_deactivates_socket) {
  Channel ws;
  SharedClient client;
  assertTrue(handshake(ws, client));
  client.clearOutput();
  client.state->failAfter = 3;
  ws.send(0, "hello");
  assertFalse(ws.connected(0));
  assertEqual((size_t)3, client.state->output.size());
}

unittest(rooms_target_broadcast_and_reset_when_slot_is_reused) {
  Channel ws;
  SharedClient first, second, replacement, full;
  int room = ws.defineRoom("lobby");
  assertEqual(0, room);
  assertTrue(handshake(ws, first));
  assertTrue(handshake(ws, second));
  assertFalse(handshake(ws, full));
  ws.join(0, room);
  first.clearOutput();
  second.clearOutput();
  ws.to(room, "hi");
  assertEqual((size_t)4, first.state->output.size());
  assertTrue(second.state->output.empty());
  ws.leave(0, room);
  assertFalse(ws.inRoom(0, room));
  ws.join(0, room);
  ws.close(0);
  assertTrue(handshake(ws, replacement));
  assertFalse(ws.inRoom(0, room));
  replacement.clearOutput();
  ws.broadcast("all");
  assertEqual((size_t)5, replacement.state->output.size());
  assertEqual((size_t)5, second.state->output.size());
}

unittest(room_names_resolve_by_content_and_reuse_capacity) {
  Channel ws;
  const Channel& rooms = ws;
  char name[] = "chat";
  char sameName[] = "chat";
  assertEqual(-1, rooms.findRoom(nullptr));
  assertEqual(-1, rooms.findRoom("chat"));
  assertEqual(-1, ws.defineRoom(nullptr));
  assertEqual(0, ws.defineRoom(name));
  assertEqual(0, rooms.findRoom(sameName));
  assertEqual(0, ws.defineRoom(sameName));
  assertEqual(1, ws.defineRoom("Chat"));
  assertEqual(1, rooms.findRoom("Chat"));
  assertEqual(-1, rooms.findRoom("CHAT"));
  assertEqual(0, ws.defineRoom(sameName));
  assertEqual(1, ws.defineRoom("Chat"));
  assertEqual(-1, ws.defineRoom("full"));
  assertEqual(-1, ws.defineRoom(nullptr));
}

unittest(empty_room_names_are_valid_and_reuse_the_same_room) {
  Channel ws;
  char empty[] = "";
  assertEqual(-1, ws.findRoom(empty));
  assertEqual(0, ws.defineRoom(empty));
  assertEqual(0, ws.findRoom(""));
  assertEqual(0, ws.defineRoom(""));
  assertEqual(1, ws.defineRoom("chat"));
  assertEqual(0, ws.defineRoom(""));
}

unittest(named_rooms_target_members_with_text_and_binary_frames) {
  Channel ws;
  SharedClient first, second;
  char chat[] = "chat";
  assertEqual(0, ws.defineRoom(chat));
  assertEqual(1, ws.defineRoom("other"));
  assertTrue(handshake(ws, first));
  assertTrue(handshake(ws, second));
  ws.join(0, "chat");
  ws.join(1, "other");
  assertTrue(ws.inRoom(0, "chat"));
  assertTrue(ws.inRoom(1, "other"));
  assertFalse(ws.inRoom(1, "chat"));
  first.clearOutput();
  second.clearOutput();

  ws.to("chat", "hi");
  std::vector<uint8_t> expected = {0x81, 2, 'h', 'i'};
  assertTrue(first.state->output == expected);
  assertTrue(second.state->output.empty());

  first.clearOutput();
  const uint8_t text[] = {'a', 0, 'b'};
  ws.to("chat", text, sizeof(text));
  expected = {0x81, 3, 'a', 0, 'b'};
  assertTrue(first.state->output == expected);
  assertTrue(second.state->output.empty());

  first.clearOutput();
  const uint8_t binary[] = {0, 0xFF, 0x80};
  ws.to("other", binary, sizeof(binary), true);
  expected = {0x82, 3, 0, 0xFF, 0x80};
  assertTrue(first.state->output.empty());
  assertTrue(second.state->output == expected);

  second.clearOutput();
  ws.leave(0, "chat");
  assertFalse(ws.inRoom(0, "chat"));
  ws.to("chat", "gone");
  assertTrue(first.state->output.empty());
  assertTrue(second.state->output.empty());

  ws.join(0, 0);
  assertTrue(ws.inRoom(0, 0));
  ws.to(0, "index");
  expected = {0x81, 5, 'i', 'n', 'd', 'e', 'x'};
  assertTrue(first.state->output == expected);
  first.clearOutput();
  ws.to(0, binary, sizeof(binary), true);
  expected = {0x82, 3, 0, 0xFF, 0x80};
  assertTrue(first.state->output == expected);
  ws.leave(0, 0);
  assertFalse(ws.inRoom(0, 0));
}

unittest(unknown_and_null_room_names_do_not_create_rooms_or_change_membership) {
  Channel ws;
  SharedClient client;
  assertEqual(0, ws.defineRoom("chat"));
  assertTrue(handshake(ws, client));
  ws.join(0, "chat");
  client.clearOutput();
  const char* names[] = {"missing", nullptr};
  const uint8_t data[] = {1, 0, 2};
  for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
    ws.join(0, names[i]);
    ws.leave(0, names[i]);
    assertFalse(ws.inRoom(0, names[i]));
    assertTrue(ws.inRoom(0, "chat"));
    ws.to(names[i], "ignored");
    ws.to(names[i], data, sizeof(data));
    ws.to(names[i], data, sizeof(data), true);
    assertTrue(client.state->output.empty());
    assertEqual(-1, ws.findRoom(names[i]));
  }
  assertEqual(1, ws.defineRoom("other"));
  assertEqual(-1, ws.defineRoom("full"));
}

unittest(heartbeat_tracks_one_outstanding_ping_and_clock_rollover) {
  Channel ws;
  SharedClient client;
  wsNow = 0xFFFFFF00UL;
  assertTrue(handshake(ws, client));
  client.clearOutput();
  wsNow = (uint32_t)(wsNow + WS_PING_INTERVAL_MS);
  ws.poll();
  std::vector<uint8_t> ping = {0x89, 0};
  assertTrue(client.state->output == ping);
  wsNow = (uint32_t)(wsNow + WS_PING_INTERVAL_MS);
  ws.poll();
  assertTrue(client.state->output == ping);
  client.feed(frame(10, {}));
  ws.poll();
  assertTrue(ws.connected(0));
  client.clearOutput();
  wsNow = (uint32_t)(wsNow + WS_PING_INTERVAL_MS);
  ws.poll();
  assertTrue(client.state->output == ping);
  wsNow = (uint32_t)(wsNow + WS_PING_TIMEOUT_MS);
  ws.poll();
  assertFalse(ws.connected(0));
}

unittest(queued_pong_is_processed_before_heartbeat_timeout) {
  Channel ws;
  SharedClient client;
  wsNow = 0;
  assertTrue(handshake(ws, client));
  client.clearOutput();
  wsNow += WS_PING_INTERVAL_MS;
  ws.poll();
  assertTrue(ws.connected(0));
  client.feed(frame(10, {}));
  wsNow += WS_PING_TIMEOUT_MS;
  ws.poll();
  assertTrue(ws.connected(0));
  assertEqual(0, client.available());
}

unittest(message_stream_peek_and_exhaustion) {
  uint8_t data[] = {'A', 'B'};
  WsMessage msg(data, sizeof(data), WsMessage::TEXT, 7);
  assertEqual(2, msg.available());
  assertEqual((int)'A', msg.peek());
  assertEqual((int)'A', msg.read());
  assertEqual((int)'B', msg.read());
  assertEqual(-1, msg.peek());
  assertEqual(-1, msg.read());
  assertEqual(0, msg.available());
  assertEqual((size_t)0, msg.write('x'));
  assertTrue(msg.isText());
  assertEqual(7, msg.socketId());
}

unittest_main()
