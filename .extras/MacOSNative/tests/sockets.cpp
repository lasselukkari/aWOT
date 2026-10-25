#include <aWOT.h>
#include <assert.h>
#include <signal.h>
#include <sys/wait.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <utility>

#include "MacOSSocket.h"

#define CHECK(condition)                                                   \
  do {                                                                     \
    if (!(condition)) {                                                    \
      std::fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
      std::abort();                                                        \
    }                                                                      \
  } while (false)

#include "../../NativeTests/shared-clients.h"

namespace {

using Clock = std::chrono::steady_clock;

long elapsedMillis(Clock::time_point start) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() -
                                                               start)
      .count();
}

struct ClientPair {
  using ClientType = MacOSClient;
  MacOSClient client;
  MacOSClient peer;

  void waitForInput(size_t length);

  ClientPair() {
    int sockets[2];
    CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    client = MacOSClient(sockets[0]);
    peer = MacOSClient(sockets[1]);
    CHECK(client && peer);
  }
};

void waitReadable(int socket) {
  pollfd descriptor = {socket, POLLIN, 0};
  CHECK(poll(&descriptor, 1, 1000) == 1);
  CHECK(descriptor.revents & (POLLIN | POLLHUP));
}

void ClientPair::waitForInput(size_t) { waitReadable(client.getSocket()); }

void checkClosed(int socket) {
  CHECK(fcntl(socket, F_GETFD) == -1);
  CHECK(errno == EBADF);
}

void testReadsAndTimeouts() {
  ClientPair pair;
  uint8_t byte;
  Clock::time_point start = Clock::now();
  CHECK(pair.client.available() == 0);
  CHECK(pair.client.read() == -1);
  CHECK(pair.client.read(&byte, 1) == 0);
  CHECK(pair.client.peek() == -1);
  CHECK(pair.client.read(&byte, 0) == 0);
  CHECK(pair.client.connected());
  CHECK(elapsedMillis(start) < 500);

  pair.client.setTimeout(30);
  start = Clock::now();
  CHECK(pair.client.readBytes(&byte, 1) == 0);
  CHECK(elapsedMillis(start) >= 20);
  CHECK(elapsedMillis(start) < 500);

  CHECK(pair.peer.write((uint8_t)'a') == 1);
  waitReadable(pair.client.getSocket());
  CHECK(pair.client.available() > 0);
  CHECK(pair.client.peek() == 'a');
  CHECK(pair.client.read() == 'a');
  CHECK(pair.client.read() == -1);
  CHECK(pair.client.connected());

  pair.peer.stop();
  waitReadable(pair.client.getSocket());
  CHECK(pair.client.read() == -1);
  CHECK(!pair.client.connected());
}

void testClientOwnership() {
  int sockets[2];
  CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
  {
    MacOSClient client(sockets[0]);
    close(sockets[1]);
    // Match HelloWorld's path that skips handling a disconnected client.
    CHECK(!client.connected());
  }
  checkClosed(sockets[0]);

  ClientPair first;
  ClientPair second;
  int firstSocket = first.client.getSocket();
  int secondSocket = second.client.getSocket();
  {
    MacOSClient moved(std::move(first.client));
    CHECK(first.client.getSocket() == -1);
    CHECK(!first.client.connected());
    first.client.stop();
    CHECK(fcntl(firstSocket, F_GETFD) >= 0);

    moved = std::move(second.client);
    checkClosed(firstSocket);
    CHECK(second.client.getSocket() == -1);
    CHECK(moved.getSocket() == secondSocket);
  }
  checkClosed(secondSocket);
}

void testBufferedReads() {
  ClientPair pair;
  uint8_t payload[2500];
  for (size_t i = 0; i < sizeof(payload); ++i) payload[i] = i % 251;
  CHECK(pair.peer.write(payload, sizeof(payload)) == sizeof(payload));
  pair.peer.stop();
  waitReadable(pair.client.getSocket());

  CHECK(pair.client.read() == payload[0]);
  CHECK(pair.client.available() > 1);
  CHECK(pair.client.connected());
  CHECK(pair.client.peek() == payload[1]);
  CHECK(pair.client.peek() == payload[1]);
  uint8_t block[37];
  CHECK(pair.client.read(block, 0) == 0);

  // Alternate byte and bulk reads across several receive-buffer boundaries.
  Client* raw = &pair.client;
  size_t offset = 1;
  while (offset < sizeof(payload)) {
    CHECK(raw->connected());
    CHECK(raw->peek() == payload[offset]);
    CHECK(raw->read() == payload[offset++]);
    size_t remaining = sizeof(payload) - offset;
    size_t count = remaining < sizeof(block) ? remaining : sizeof(block);
    if (count) {
      const int received = raw->read(block, count);
      CHECK(received > 0 && static_cast<size_t>(received) <= count);
      CHECK(std::memcmp(block, payload + offset, received) == 0);
      offset += received;
    }
  }
  CHECK(raw->available() == 0);
  CHECK(raw->read() == -1);
  CHECK(raw->read(block, sizeof(block)) == 0);
  CHECK(raw->peek() == -1);
  CHECK(!raw->connected());
}

void testBufferedMovesAndStop() {
  ClientPair pair;
  const uint8_t payload[] = "abcdef";
  CHECK(pair.peer.write(payload, sizeof(payload) - 1) == sizeof(payload) - 1);
  waitReadable(pair.client.getSocket());
  CHECK(pair.client.read() == 'a');
  CHECK(pair.client.available() == 5);
  pair.peer.stop();

  MacOSClient moved(std::move(pair.client));
  CHECK(!pair.client.connected());
  CHECK(pair.client.available() == 0);
  CHECK(pair.client.read() == -1);
  CHECK(moved.connected());
  CHECK(moved.available() == 5);
  CHECK(moved.read() == 'b');

  ClientPair replacement;
  CHECK(replacement.peer.write((const uint8_t*)"stale", 5) == 5);
  waitReadable(replacement.client.getSocket());
  CHECK(replacement.client.peek() == 's');
  replacement.client = std::move(moved);
  CHECK(!moved.connected());
  CHECK(moved.available() == 0);
  CHECK(replacement.client.available() == 4);
  CHECK(replacement.client.read() == 'c');
  CHECK(replacement.client.connected());
  replacement.client.stop();
  CHECK(replacement.client.available() == 0);
  CHECK(replacement.client.read() == -1);
  CHECK(replacement.client.peek() == -1);
  CHECK(!replacement.client.connected());
}

void checkWritesAfterPeerClose(MacOSClient& client, MacOSClient& peer) {
  peer.stop();
  waitReadable(client.getSocket());
  // A child lets the assertion report SIGPIPE even if the test runner ignores
  // it.
  pid_t child = fork();
  CHECK(child >= 0);
  if (child == 0) {
    signal(SIGPIPE, SIG_DFL);
    alarm(3);
    for (int attempt = 0; attempt < 20; ++attempt) {
      if (client.write((uint8_t)'x') == 0) {
        _exit(client.connected() ? 1 : 0);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    _exit(2);
  }
  int status;
  CHECK(waitpid(child, &status, 0) == child);
  CHECK(WIFEXITED(status));
  CHECK(WEXITSTATUS(status) == 0);
}

void testSigpipe() {
  ClientPair wrapped;
  checkWritesAfterPeerClose(wrapped.client, wrapped.peer);

  MacOSServer server(0);
  CHECK(server.begin());
  MacOSClient outgoing;
  CHECK(outgoing.connect(IPAddress(127, 0, 0, 1), server.port()) == 1);
  MacOSClient accepted = server.available(1000);
  CHECK(accepted);
  checkWritesAfterPeerClose(accepted, outgoing);

  CHECK(outgoing.connect("127.0.0.1", server.port()) == 1);
  accepted = server.available(1000);
  CHECK(accepted);
  checkWritesAfterPeerClose(outgoing, accepted);
}

void testConnectAndReconnect() {
  MacOSServer server(0);
  CHECK(server.begin());
  CHECK(server.port() != 0);

  MacOSClient outgoing;
  CHECK(outgoing.connect(IPAddress(127, 0, 0, 1), server.port()) == 1);
  MacOSClient first = server.available(1000);
  CHECK(first);
  CHECK(!(fcntl(first.getSocket(), F_GETFL) & O_NONBLOCK));
  CHECK(outgoing.write((uint8_t)'a') == 1);
  waitReadable(first.getSocket());
  CHECK(first.read() == 'a');

  CHECK(first.write((const uint8_t*)"old", 3) == 3);
  waitReadable(outgoing.getSocket());
  CHECK(outgoing.peek() == 'o');
  CHECK(outgoing.available() == 3);

  MacOSClient oldConnection(outgoing);
  CHECK(outgoing.connect("127.0.0.1", server.port()) == 1);
  CHECK(!oldConnection.connected());
  CHECK(oldConnection.getSocket() == -1);
  CHECK(oldConnection.available() == 0);
  CHECK(outgoing.available() == 0);
  CHECK(outgoing.read() == -1);
  waitReadable(first.getSocket());
  CHECK(first.read() == -1);
  CHECK(!first.connected());
  MacOSClient second = server.available(1000);
  CHECK(second);
  CHECK(second.write((uint8_t)'b') == 1);
  waitReadable(outgoing.getSocket());
  CHECK(outgoing.read() == 'b');

  CHECK(second.write((const uint8_t*)"old", 3) == 3);
  waitReadable(outgoing.getSocket());
  CHECK(outgoing.peek() == 'o');

  server.stop();
  CHECK(outgoing.connect(IPAddress(127, 0, 0, 1), server.port()) == 0);
  CHECK(outgoing.getSocket() == -1);
  CHECK(!outgoing.connected());
  CHECK(!outgoing);
  CHECK(outgoing.available() == 0);
  CHECK(outgoing.read() == -1);
  waitReadable(second.getSocket());
  CHECK(!second.connected());

  CHECK(server.begin());
  CHECK(outgoing.connect("127.0.0.1", server.port()) == 1);
  second = server.available(1000);
  CHECK(second);
  CHECK(outgoing.connect("not-an-ip", server.port()) == 0);
  CHECK(outgoing.getSocket() == -1);
  CHECK(!outgoing.connected());
  waitReadable(second.getSocket());
  CHECK(!second.connected());
}

void testServerOwnershipAndWaiting() {
  uint16_t port;
  {
    MacOSServer server(0);
    CHECK(server.begin());
    port = server.port();
    CHECK(server.begin());

    MacOSServer conflict(port);
    CHECK(!conflict.begin());
    CHECK(!conflict.available());

    Clock::time_point start = Clock::now();
    CHECK(!server.available());
    CHECK(elapsedMillis(start) < 500);
    start = Clock::now();
    CHECK(!server.available(75));
    CHECK(elapsedMillis(start) >= 50);
    CHECK(elapsedMillis(start) < 1000);

    start = Clock::now();
    std::thread sender([port]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      MacOSClient client;
      CHECK(client.connect("127.0.0.1", port) == 1);
      CHECK(client.write((uint8_t)'w') == 1);
    });
    MacOSClient accepted = server.available(-1);
    sender.join();
    CHECK(accepted);
    CHECK(elapsedMillis(start) >= 25);
    CHECK(elapsedMillis(start) < 1000);
    waitReadable(accepted.getSocket());
    CHECK(accepted.read() == 'w');

    server.stop();
    CHECK(conflict.begin());
  }
  MacOSServer replacement(port);
  CHECK(replacement.begin());
}

void testAppTimeout() {
  ClientPair pair;
  const char request[] = "GET / HTTP/1.1\r\nHost: localhost\r\n";
  CHECK(pair.peer.write((const uint8_t*)request, sizeof(request) - 1) ==
        sizeof(request) - 1);
  App app;
  app.setTimeout(40);
  Clock::time_point start = Clock::now();
  App::ProcessResult result = app.process(&pair.client);
  CHECK(elapsedMillis(start) >= 30);
  CHECK(elapsedMillis(start) < 1000);
  CHECK(!result.keepAlive && !result.responseOpen);
  CHECK(pair.client.getSocket() == -1);
  waitReadable(pair.peer.getSocket());
  uint8_t response[1024] = {};
  CHECK(pair.peer.read(response, sizeof(response) - 1) > 0);
  CHECK(std::strstr((const char*)response, "HTTP/1.1 408 Request Timeout"));
}

void rawBodyHandler(Request& request, Response& response) {
  // Parsing headers can read ahead into the body; raw Client access must still
  // see those bytes, and the route can mix its byte and bulk read interfaces.
  Client* raw = request.client();
  CHECK(raw->peek() == 'h');
  CHECK(raw->read() == 'h');
  uint8_t body[4];
  CHECK(raw->readBytes(body, sizeof(body)) == sizeof(body));
  CHECK(std::memcmp(body, "ello", sizeof(body)) == 0);
  response.print("raw body received");
}

void testAppRawBody() {
  ClientPair pair;
  const char request[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nhello";
  CHECK(pair.peer.write((const uint8_t*)request, sizeof(request) - 1) ==
        sizeof(request) - 1);
  App app;
  app.post("/", rawBodyHandler);
  app.process(&pair.client);
  waitReadable(pair.peer.getSocket());
  uint8_t response[1024] = {};
  CHECK(pair.peer.read(response, sizeof(response) - 1) > 0);
  CHECK(std::strstr((const char*)response, "raw body received"));
}

}  // namespace

int main() {
  // Bound failures in blocking reads, connects or accept to keep regressions
  // safe.
  alarm(15);
  testReadsAndTimeouts();
  testClientOwnership();
  testBufferedReads();
  testBufferedMovesAndStop();
  native_tests::testSharedClients<ClientPair>();
  testSigpipe();
  testConnectAndReconnect();
  testServerOwnershipAndWaiting();
  testAppTimeout();
  testAppRawBody();
  alarm(0);
  std::puts("Socket regression tests passed");
  return 0;
}
