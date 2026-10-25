#ifndef AWOT_NATIVE_SHARED_CLIENT_TESTS_H_
#define AWOT_NATIVE_SHARED_CLIENT_TESTS_H_

#include <SseChannel.h>
#include <WsChannel.h>

#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <utility>

namespace native_tests {

inline std::string readUntilClosed(Client& client) {
  std::string output;
  const auto start = std::chrono::steady_clock::now();
  uint8_t buffer[256];
  while (client.connected()) {
    const int count = client.read(buffer, sizeof(buffer));
    if (count > 0) output.append(reinterpret_cast<const char*>(buffer), count);
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::seconds(2));
    if (count <= 0) std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return output;
}

template <typename Pair>
void testCopiesShareBufferedReads() {
  using NativeClient = typename Pair::ClientType;
  Pair pair;
  CHECK(pair.peer.write(reinterpret_cast<const uint8_t*>("abcdef"), 6) == 6);
  pair.waitForInput(6);
  CHECK(pair.client.peek() == 'a');

  NativeClient retained;
  {
    NativeClient copy(pair.client);
    CHECK(copy.getSocket() == pair.client.getSocket());
    CHECK(copy.read() == 'a');
    CHECK(pair.client.read() == 'b');
    uint8_t bytes[2];
    CHECK(copy.read(bytes, sizeof(bytes)) == 2);
    CHECK(std::memcmp(bytes, "cd", 2) == 0);
    CHECK(pair.client.available() == 2);
    retained = copy;
  }
  pair.client = NativeClient();
  CHECK(retained.connected());
  CHECK(retained.read() == 'e');
  CHECK(retained.read() == 'f');
  CHECK(retained.write(static_cast<uint8_t>('x')) == 1);
  pair.peer.setTimeout(1000);
  uint8_t byte;
  CHECK(pair.peer.readBytes(&byte, 1) == 1);
  CHECK(byte == 'x');

  // Releasing the last local handle must close the peer's connection.
  retained = NativeClient();
  CHECK(readUntilClosed(pair.peer).empty());
}

template <typename Pair>
void testAssignmentPreservesOtherOwners() {
  using NativeClient = typename Pair::ClientType;
  Pair first;
  Pair second;
  {
    NativeClient retained(first.client);
    // Copy assignment releases just this handle's old connection.
    first.client = second.client;
    CHECK(retained.connected());
    CHECK(retained.write(static_cast<uint8_t>('a')) == 1);
    first.peer.setTimeout(1000);
    uint8_t byte;
    CHECK(first.peer.readBytes(&byte, 1) == 1);
    CHECK(byte == 'a');

    // Move assignment must also leave other owners of the old connection live.
    first.client = std::move(retained);
    CHECK(second.client.connected());
    CHECK(second.client.write(static_cast<uint8_t>('b')) == 1);
    second.peer.setTimeout(1000);
    CHECK(second.peer.readBytes(&byte, 1) == 1);
    CHECK(byte == 'b');
  }
  CHECK(first.client.connected());
  first.client = NativeClient();
  CHECK(readUntilClosed(first.peer).empty());
}

template <typename Pair>
void testStopClosesAllCopies() {
  using NativeClient = typename Pair::ClientType;
  Pair pair;
  NativeClient copy(pair.client);
  CHECK(pair.peer.write(reinterpret_cast<const uint8_t*>("stale"), 5) == 5);
  pair.waitForInput(5);
  CHECK(copy.peek() == 's');
  copy.stop();
  CHECK(!pair.client.connected());
  CHECK(!pair.client);
  CHECK(pair.client.available() == 0);
  CHECK(pair.client.read() == -1);
  CHECK(pair.client.peek() == -1);
  CHECK(pair.client.write(static_cast<uint8_t>('x')) == 0);
  CHECK(pair.client.getSocket() == NativeClient().getSocket());
  CHECK(readUntilClosed(pair.peer).empty());

  // A closed shared state must never close a descriptor reused by a new pair.
  Pair replacement;
  pair.client.stop();
  copy.stop();
  CHECK(replacement.client.connected());
  CHECK(replacement.peer.connected());
}

template <typename Pair>
void testSseRetainsNativeClient() {
  using NativeClient = typename Pair::ClientType;
  Pair pair;
  awot::SseChannel<NativeClient, 1> events;
  App app;
  app.get("/events", [&events](Request&, Response& res) { events.open(res); });
  events.onConnect([&events](int id) { events.send(id, "hello"); });
  const std::string input = "GET /events HTTP/1.1\r\nHost: localhost\r\n\r\n";
  CHECK(pair.peer.write(reinterpret_cast<const uint8_t*>(input.data()),
                        input.size()) == input.size());
  pair.waitForInput(input.size());
  {
    NativeClient local(pair.client);
    const auto result = app.process(&local);
    CHECK(result.responseOpen && !result.keepAlive);
    CHECK(events.add(local) == 0);
  }
  pair.client = NativeClient();
  events.broadcast("later");
  events.close(0);
  CHECK(readUntilClosed(pair.peer) ==
        "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\n"
        "Cache-Control: no-cache\r\nConnection: close\r\n\r\n"
        "data: hello\n\ndata: later\n\n");
}

template <typename Pair>
void testWebSocketRetainsBufferedFrame() {
  using NativeClient = typename Pair::ClientType;
  Pair pair;
  awot::WsChannel<NativeClient, 1> sockets;
  App app;
  bool finalized = false;
  int messages = 0;
  app.get("/ws", [&sockets](Request& req, Response& res) {
    CHECK(sockets.upgrade(req, res));
  });
  app.finally([&finalized](Request& req, Response& res) {
    CHECK(req.client()->connected());
    CHECK(res.headersSent());
    finalized = true;
  });
  sockets.onMessage([&](awot::WsMessage& message) {
    CHECK(message.isText());
    CHECK(message.length() == 2);
    CHECK(std::memcmp(message.data(), "hi", 2) == 0);
    ++messages;
    sockets.send(message.socketId(), "echo");
  });

  // Queue the first masked frame with the HTTP handshake. The HTTP parser
  // reads it into the native receive buffer before the channel copies it.
  const uint8_t frame[] = {0x81, 0x82, 1, 2, 3, 4, 'h' ^ 1, 'i' ^ 2};
  std::string input =
      "GET /ws HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\n"
      "Connection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
      "Sec-WebSocket-Version: 13\r\n\r\n";
  input.append(reinterpret_cast<const char*>(frame), sizeof(frame));
  CHECK(pair.peer.write(reinterpret_cast<const uint8_t*>(input.data()),
                        input.size()) == input.size());
  pair.waitForInput(input.size());
  {
    NativeClient local(pair.client);
    char upgrade[16], connection[16], key[32], version[8];
    Request::Header headers[] = {{"Upgrade", upgrade},
                                 {"Connection", connection},
                                 {"Sec-WebSocket-Key", key},
                                 {"Sec-WebSocket-Version", version}};
    const auto result = app.process(&local, headers);
    CHECK(result.responseOpen && !result.keepAlive);
    CHECK(finalized);
    CHECK(local.available() == sizeof(frame));
  }
  pair.client = NativeClient();
  sockets.poll();
  CHECK(messages == 1);
  sockets.close(0);
  CHECK(readUntilClosed(pair.peer) ==
        "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n"
        "\x81\x04"
        "echo"
        "\x88\x02\x03\xe8");
}

template <typename Pair>
void testSharedClients() {
  testCopiesShareBufferedReads<Pair>();
  testAssignmentPreservesOtherOwners<Pair>();
  testStopClosesAllCopies<Pair>();
  testSseRetainsNativeClient<Pair>();
  testWebSocketRetainsBufferedFrame<Pair>();
}

}  // namespace native_tests

#endif
