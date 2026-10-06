#include <ArduinoUnitTests.h>

#include "../../src/aWOT.h"
#include "mocks/SharedClient.h"

static unsigned long wsNow;
static unsigned long wsMillis() { return wsNow; }
#define millis wsMillis
#include "../../src/WsChannel.h"
#undef millis

using namespace awot;
typedef WsChannel<SharedClient, 2, 2> Channel;
static Channel* upgrading;
static bool upgradeResult;
static int connects, disconnects, messageCount, lastSocket;
static bool lastBinary;
static std::vector<uint8_t> lastData;

static void upgradeHandler(Request& req, Response& res) {
  upgradeResult = upgrading->upgrade(req, res);
  if (!upgradeResult) res.sendStatus(400);
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
                      const std::string& input = request()) {
  App app;
  upgrading = &ws;
  upgradeResult = false;
  app.use(upgradeHandler);
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

static std::vector<uint8_t> frame(uint8_t opcode,
                                  const std::vector<uint8_t>& data) {
  std::vector<uint8_t> result;
  result.push_back(0x80 | opcode);
  if (data.size() <= 125) {
    result.push_back(0x80 | (uint8_t)data.size());
  } else {
    result.push_back(0xFE);
    result.push_back((uint8_t)(data.size() >> 8));
    result.push_back((uint8_t)data.size());
  }
  const uint8_t mask[] = {0x37, 0xFA, 0x21, 0x3D};
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
  assertTrue(closesWith({0x01, 0x80}, 1003));
  assertTrue(closesWith({0x80, 0x80}, 1003));
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
