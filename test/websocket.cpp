#include <aWOT.h>

#include "ArduinoUnitTests.h"
#include "mocks/MockClient.h"

using namespace awot;

// Test handlers
static int lastMessageClientId = -1;
static bool lastMessageBinary = false;
static size_t lastMessageLen = 0;
static uint8_t lastMessageData[64];

void testMessageHandler(WebSocketMessage& msg) {
  lastMessageClientId = msg.clientId();
  lastMessageBinary = msg.isBinary();
  lastMessageLen = 0;

  // Read message using Stream interface
  while (msg.available() && lastMessageLen < sizeof(lastMessageData)) {
    int byte = msg.read();
    if (byte >= 0) {
      lastMessageData[lastMessageLen++] = (uint8_t)byte;
    }
  }
}

static int lastConnectedClientId = -1;
void testConnectHandler(int clientId) { lastConnectedClientId = clientId; }

static int lastDisconnectedClientId = -1;
void testDisconnectHandler(int clientId) {
  lastDisconnectedClientId = clientId;
}

// RFC 6455 test vector for Sec-WebSocket-Accept
unittest(handshake_accept_key) {
  MockClient client;
  WebSocket ws;

  // The RFC 6455 example key
  const char* testKey = "dGhlIHNhbXBsZSBub25jZQ==";

  bool upgraded = ws.upgrade(&client, testKey);

  assertTrue(upgraded);
  assertTrue(client.writeBufferContains("HTTP/1.1 101 Switching Protocols"));
  assertTrue(client.writeBufferContains("Upgrade: websocket"));
  assertTrue(client.writeBufferContains("Connection: Upgrade"));
  // Expected accept key from RFC 6455
  assertTrue(client.writeBufferContains(
      "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo="));
}

unittest(client_count_after_upgrade) {
  MockClient client1, client2;
  WebSocket ws;

  assertEqual(0, ws.clientCount());

  ws.upgrade(&client1, "key1");
  assertEqual(1, ws.clientCount());

  ws.upgrade(&client2, "key2");
  assertEqual(2, ws.clientCount());
}

unittest(client_connected_status) {
  MockClient client;
  WebSocket ws;

  assertFalse(ws.connected(0));

  ws.upgrade(&client, "key");

  assertTrue(ws.connected(0));
  assertFalse(ws.connected(1));
  assertFalse(ws.connected(-1));
  assertFalse(ws.connected(99));
}

unittest(connect_handler_called) {
  MockClient client;
  WebSocket ws;

  lastConnectedClientId = -1;
  ws.onConnect(testConnectHandler);
  ws.upgrade(&client, "key");

  assertEqual(0, lastConnectedClientId);
}

unittest(disconnect_on_close_frame) {
  MockClient client;
  WebSocket ws;

  ws.onDisconnect(testDisconnectHandler);
  ws.upgrade(&client, "key");

  assertTrue(ws.connected(0));

  // Close frame: FIN=1, opcode=8, mask=1, len=0
  uint8_t closeFrame[] = {0x88, 0x80, 0x00, 0x00, 0x00, 0x00};
  client.setReadBuffer(closeFrame, sizeof(closeFrame));

  ws.poll();

  assertFalse(ws.connected(0));
  assertEqual(0, lastDisconnectedClientId);
}

unittest(text_frame_with_stream_read) {
  MockClient client;
  WebSocket ws;

  ws.onMessage(testMessageHandler);
  ws.upgrade(&client, "key");
  client.clearWriteBuffer();

  // Text frame with "Hello" masked
  // Mask: 0x37, 0xfa, 0x21, 0x3d
  // "Hello" = 0x48, 0x65, 0x6c, 0x6c, 0x6f
  // Masked: 0x48^0x37=0x7f, 0x65^0xfa=0x9f, 0x6c^0x21=0x4d, 0x6c^0x3d=0x51,
  // 0x6f^0x37=0x58
  uint8_t textFrame[] = {0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d,
                         0x7f, 0x9f, 0x4d, 0x51, 0x58};
  client.setReadBuffer(textFrame, sizeof(textFrame));

  ws.poll();

  assertEqual(0, lastMessageClientId);
  assertFalse(lastMessageBinary);
  assertEqual((size_t)5, lastMessageLen);
  assertEqual('H', (char)lastMessageData[0]);
  assertEqual('e', (char)lastMessageData[1]);
  assertEqual('l', (char)lastMessageData[2]);
  assertEqual('l', (char)lastMessageData[3]);
  assertEqual('o', (char)lastMessageData[4]);
}

unittest(binary_frame_detected) {
  MockClient client;
  WebSocket ws;

  ws.onMessage(testMessageHandler);
  ws.upgrade(&client, "key");
  client.clearWriteBuffer();

  // Binary frame (opcode=2) with payload [0x01, 0x02]
  // Mask: 0x11, 0x22, 0x33, 0x44
  // Payload masked: 0x01^0x11=0x10, 0x02^0x22=0x20
  uint8_t binaryFrame[] = {0x82, 0x82, 0x11, 0x22, 0x33, 0x44, 0x10, 0x20};
  client.setReadBuffer(binaryFrame, sizeof(binaryFrame));

  ws.poll();

  assertEqual(0, lastMessageClientId);
  assertTrue(lastMessageBinary);
  assertEqual((size_t)2, lastMessageLen);
  assertEqual((uint8_t)0x01, lastMessageData[0]);
  assertEqual((uint8_t)0x02, lastMessageData[1]);
}

unittest(ping_pong_response) {
  MockClient client;
  WebSocket ws;

  ws.upgrade(&client, "key");
  client.clearWriteBuffer();

  // Ping frame with "test" payload masked
  // Mask: 0x00, 0x00, 0x00, 0x00 (zero mask for simplicity)
  uint8_t pingFrame[] = {0x89, 0x84, 0x00, 0x00, 0x00,
                         0x00, 't',  'e',  's',  't'};
  client.setReadBuffer(pingFrame, sizeof(pingFrame));

  ws.poll();

  // Check pong response
  const uint8_t* written = client.getWriteBuffer();
  size_t writtenLen = client.getWriteLength();

  assertTrue(writtenLen >= 6);
  assertEqual((uint8_t)0x8A, written[0]);  // Pong opcode
  assertEqual((uint8_t)4, written[1]);     // Length
  assertEqual('t', (char)written[2]);
  assertEqual('e', (char)written[3]);
  assertEqual('s', (char)written[4]);
  assertEqual('t', (char)written[5]);
}

unittest(room_join_and_leave) {
  MockClient client1, client2, client3;
  WebSocket ws;

  ws.upgrade(&client1, "key1");
  ws.upgrade(&client2, "key2");
  ws.upgrade(&client3, "key3");

  ws.join(0, "room1");
  ws.join(1, "room1");
  // client3 not in room1

  client1.clearWriteBuffer();
  client2.clearWriteBuffer();
  client3.clearWriteBuffer();

  ws.toText("room1", "Hi");

  assertTrue(client1.getWriteLength() > 0);
  assertTrue(client2.getWriteLength() > 0);
  assertEqual((size_t)0, client3.getWriteLength());

  // Leave room
  ws.leave(0, "room1");
  client1.clearWriteBuffer();
  client2.clearWriteBuffer();

  ws.toText("room1", "Bye");

  assertEqual((size_t)0, client1.getWriteLength());
  assertTrue(client2.getWriteLength() > 0);
}

unittest(broadcast_to_all) {
  MockClient client1, client2;
  WebSocket ws;

  ws.upgrade(&client1, "key1");
  ws.upgrade(&client2, "key2");

  client1.clearWriteBuffer();
  client2.clearWriteBuffer();

  ws.broadcastText("Test");

  assertTrue(client1.getWriteLength() > 0);
  assertTrue(client2.getWriteLength() > 0);
}

unittest(send_to_specific_client) {
  MockClient client1, client2;
  WebSocket ws;

  ws.upgrade(&client1, "key1");
  ws.upgrade(&client2, "key2");

  client1.clearWriteBuffer();
  client2.clearWriteBuffer();

  ws.sendText(0, "OnlyYou");

  assertTrue(client1.getWriteLength() > 0);
  assertEqual((size_t)0, client2.getWriteLength());
}

unittest(message_stream_write_response) {
  MockClient client;
  WebSocket ws;

  bool handlerCalled = false;

  ws.onMessage([](WebSocketMessage& msg) {
    // Echo back using Stream write
    while (msg.available()) {
      uint8_t byte = msg.read();
      msg.write(byte);  // Write response
    }
  });

  ws.upgrade(&client, "key");
  client.clearWriteBuffer();

  // Send "AB" masked with zero mask
  uint8_t frame[] = {0x81, 0x82, 0x00, 0x00, 0x00, 0x00, 'A', 'B'};
  client.setReadBuffer(frame, sizeof(frame));

  ws.poll();

  // Should have sent two binary frames (one per byte)
  assertTrue(client.getWriteLength() > 0);
}

unittest_main()
