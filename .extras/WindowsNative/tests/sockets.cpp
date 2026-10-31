#include <aWOT.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

#include "WindowsSocket.h"

#define CHECK(condition)                                               \
  do {                                                                 \
    if (!(condition)) {                                                \
      std::fprintf(stderr, "%s:%d: %s (Winsock error %d)\n", __FILE__, \
                   __LINE__, #condition, WSAGetLastError());           \
      std::abort();                                                    \
    }                                                                  \
  } while (false)

#include "../../NativeTests/shared-clients.h"

static_assert(std::is_copy_constructible<WindowsClient>::value,
              "Channels retain copies of native clients");
static_assert(std::is_copy_assignable<WindowsClient>::value,
              "Channels retain copies of native clients");
static_assert(std::is_same<decltype(std::declval<WindowsClient>().getSocket()),
                           SOCKET>::value,
              "A Windows socket handle must not be truncated to int");

namespace {

using Clock = std::chrono::steady_clock;

long elapsedMillis(Clock::time_point start) {
  return static_cast<long>(
      std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() -
                                                            start)
          .count());
}

// Winsock has no socketpair(). Use real IPv4 loopback TCP sockets, and wrap
// both endpoints so the constructor and the outgoing-client path are covered
// independently of WindowsServer.
struct ClientPair {
  using ClientType = WindowsClient;
  WindowsClient client;
  WindowsClient peer;

  void waitForInput(size_t length);

  ClientPair() {
    SOCKET listener = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CHECK(listener != INVALID_SOCKET);
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    CHECK(bind(listener, reinterpret_cast<sockaddr*>(&address),
               sizeof(address)) == 0);
    CHECK(listen(listener, 1) == 0);
    int length = sizeof(address);
    CHECK(getsockname(listener, reinterpret_cast<sockaddr*>(&address),
                      &length) == 0);

    SOCKET outgoing = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CHECK(outgoing != INVALID_SOCKET);
    CHECK(::connect(outgoing, reinterpret_cast<sockaddr*>(&address),
                    sizeof(address)) == 0);
    SOCKET incoming = accept(listener, nullptr, nullptr);
    CHECK(incoming != INVALID_SOCKET);
    CHECK(closesocket(listener) == 0);
    client = WindowsClient(incoming);
    peer = WindowsClient(outgoing);
    CHECK(client && peer);
  }
};

void waitReadable(SOCKET socket) {
  CHECK(socket != INVALID_SOCKET);
  fd_set readers;
  FD_ZERO(&readers);
  FD_SET(socket, &readers);
  timeval timeout = {2, 0};
  CHECK(select(0, &readers, nullptr, nullptr, &timeout) == 1);
  CHECK(FD_ISSET(socket, &readers));
}

// TCP can split a send into several receives. Wait for test payloads to arrive
// before making assertions about a buffered read or moving unread bytes.
void waitQueued(SOCKET socket, size_t minimum) {
  Clock::time_point start = Clock::now();
  while (true) {
    u_long queued = 0;
    CHECK(ioctlsocket(socket, FIONREAD, &queued) == 0);
    if (queued >= minimum) return;
    CHECK(elapsedMillis(start) < 2000);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void ClientPair::waitForInput(size_t length) {
  waitQueued(client.getSocket(), length);
}

void checkClosed(SOCKET socket) {
  int type = 0;
  int length = sizeof(type);
  CHECK(getsockopt(socket, SOL_SOCKET, SO_TYPE, reinterpret_cast<char*>(&type),
                   &length) == SOCKET_ERROR);
  CHECK(WSAGetLastError() == WSAENOTSOCK);
}

void testAutomaticWinsockLifetime() {
  // Run before the raw-pair helper's WSAStartup. Listener and client objects
  // must initialize Winsock themselves and keep it alive independently.
  for (int attempt = 0; attempt < 2; ++attempt) {
    WindowsClient outgoing;
    WindowsClient accepted;
    {
      WindowsServer server(0);
      CHECK(server.begin());
      CHECK(outgoing.connect("127.0.0.1", server.port()) == 1);
      accepted = server.available(2000);
      CHECK(accepted);
    }
    WindowsClient copied(accepted);
    accepted = WindowsClient();
    WindowsClient moved(std::move(copied));
    CHECK(copied.getSocket() == INVALID_SOCKET);
    CHECK(accepted.getSocket() == INVALID_SOCKET);
    CHECK(outgoing.write(static_cast<uint8_t>('a')) == 1);
    waitReadable(moved.getSocket());
    CHECK(moved.read() == 'a');
    CHECK(moved.write(static_cast<uint8_t>('b')) == 1);
    waitReadable(outgoing.getSocket());
    CHECK(outgoing.read() == 'b');
  }
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

  pair.client.setTimeout(60);
  start = Clock::now();
  CHECK(pair.client.readBytes(&byte, 1) == 0);
  CHECK(elapsedMillis(start) >= 40);
  CHECK(elapsedMillis(start) < 2000);

  CHECK(pair.peer.write(static_cast<uint8_t>('a')) == 1);
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
  ClientPair disconnected;
  SOCKET disconnectedSocket = disconnected.client.getSocket();
  disconnected.peer.stop();
  waitReadable(disconnectedSocket);
  CHECK(!disconnected.client.connected());
  {
    WindowsClient client(std::move(disconnected.client));
    // A disconnected accepted client still owns its handle until destruction.
    CHECK(client.getSocket() == disconnectedSocket);
  }
  checkClosed(disconnectedSocket);

  ClientPair first;
  ClientPair second;
  SOCKET firstSocket = first.client.getSocket();
  SOCKET secondSocket = second.client.getSocket();
  {
    WindowsClient moved(std::move(first.client));
    CHECK(first.client.getSocket() == INVALID_SOCKET);
    CHECK(!first.client.connected());
    first.client.stop();
    int type = 0;
    int length = sizeof(type);
    CHECK(getsockopt(firstSocket, SOL_SOCKET, SO_TYPE,
                     reinterpret_cast<char*>(&type), &length) == 0);

    moved = std::move(second.client);
    checkClosed(firstSocket);
    CHECK(second.client.getSocket() == INVALID_SOCKET);
    CHECK(moved.getSocket() == secondSocket);
  }
  checkClosed(secondSocket);
}

void testBufferedReads() {
  ClientPair pair;
  uint8_t payload[2500];
  for (size_t i = 0; i < sizeof(payload); ++i) payload[i] = i % 251;
  CHECK(pair.peer.write(payload, sizeof(payload)) == sizeof(payload));
  waitQueued(pair.client.getSocket(), sizeof(payload));
  pair.peer.stop();

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
  waitReadable(pair.client.getSocket());
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
  waitQueued(pair.client.getSocket(), sizeof(payload) - 1);
  CHECK(pair.client.read() == 'a');
  CHECK(pair.client.available() == 5);
  pair.peer.stop();

  WindowsClient moved(std::move(pair.client));
  CHECK(!pair.client.connected());
  CHECK(pair.client.available() == 0);
  CHECK(pair.client.read() == -1);
  CHECK(moved.connected());
  CHECK(moved.available() == 5);
  CHECK(moved.read() == 'b');

  ClientPair replacement;
  CHECK(replacement.peer.write(reinterpret_cast<const uint8_t*>("stale"), 5) ==
        5);
  waitQueued(replacement.client.getSocket(), 5);
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

void checkWritesAfterPeerClose(WindowsClient& client, WindowsClient& peer) {
  peer.stop();
  waitReadable(client.getSocket());
  Clock::time_point start = Clock::now();
  while (client.write(static_cast<uint8_t>('x')) != 0) {
    CHECK(elapsedMillis(start) < 2000);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  CHECK(!client.connected());
}

void testWritesAfterPeerClose() {
  ClientPair wrapped;
  checkWritesAfterPeerClose(wrapped.client, wrapped.peer);

  WindowsServer server(0);
  CHECK(server.begin());
  WindowsClient outgoing;
  CHECK(outgoing.connect(IPAddress(127, 0, 0, 1), server.port()) == 1);
  WindowsClient accepted = server.available(2000);
  CHECK(accepted);
  checkWritesAfterPeerClose(accepted, outgoing);

  CHECK(outgoing.connect("127.0.0.1", server.port()) == 1);
  accepted = server.available(2000);
  CHECK(accepted);
  checkWritesAfterPeerClose(outgoing, accepted);
}

void testWriteBackpressureTimeout() {
  ClientPair pair;
  int bufferSize = 1024;
  CHECK(setsockopt(pair.client.getSocket(), SOL_SOCKET, SO_SNDBUF,
                   reinterpret_cast<const char*>(&bufferSize),
                   sizeof(bufferSize)) == 0);
  CHECK(setsockopt(pair.peer.getSocket(), SOL_SOCKET, SO_RCVBUF,
                   reinterpret_cast<const char*>(&bufferSize),
                   sizeof(bufferSize)) == 0);
  pair.client.setTimeout(60);

  uint8_t payload[65536];
  std::memset(payload, 'b', sizeof(payload));
  Clock::time_point start = Clock::now();
  size_t total = 0;
  bool blocked = false;
  while (total < 16 * 1024 * 1024) {
    CHECK(elapsedMillis(start) < 3000);
    Clock::time_point attemptStart = Clock::now();
    const size_t written = pair.client.write(payload, sizeof(payload));
    CHECK(written <= sizeof(payload));
    if (written == 0) {
      CHECK(elapsedMillis(attemptStart) >= 40);
      CHECK(elapsedMillis(attemptStart) < 2000);
      blocked = true;
      break;
    }
    total += written;
  }
  CHECK(blocked && total > 0);
  // A timeout caused by a peer that stops reading must preserve the socket.
  CHECK(pair.client.connected());
  waitReadable(pair.peer.getSocket());
  CHECK(pair.peer.connected());
  CHECK(pair.peer.read() == 'b');
}

void testConnectAndReconnect() {
  WindowsServer server(0);
  CHECK(server.begin());
  CHECK(server.port() != 0);

  WindowsClient outgoing;
  CHECK(outgoing.connect(IPAddress(127, 0, 0, 1), server.port()) == 1);
  WindowsClient first = server.available(2000);
  CHECK(first);
  CHECK(outgoing.write(static_cast<uint8_t>('a')) == 1);
  waitReadable(first.getSocket());
  CHECK(first.read() == 'a');

  CHECK(first.write(reinterpret_cast<const uint8_t*>("old"), 3) == 3);
  waitQueued(outgoing.getSocket(), 3);
  CHECK(outgoing.peek() == 'o');
  CHECK(outgoing.available() == 3);

  WindowsClient oldConnection(outgoing);
  CHECK(outgoing.connect("127.0.0.1", server.port()) == 1);
  CHECK(!oldConnection.connected());
  CHECK(oldConnection.getSocket() == INVALID_SOCKET);
  CHECK(oldConnection.available() == 0);
  CHECK(outgoing.available() == 0);
  CHECK(outgoing.read() == -1);
  waitReadable(first.getSocket());
  CHECK(first.read() == -1);
  CHECK(!first.connected());
  first.stop();
  WindowsClient second = server.available(2000);
  CHECK(second);
  CHECK(second.write(static_cast<uint8_t>('b')) == 1);
  waitReadable(outgoing.getSocket());
  CHECK(outgoing.read() == 'b');

  CHECK(second.write(reinterpret_cast<const uint8_t*>("old"), 3) == 3);
  waitQueued(outgoing.getSocket(), 3);
  CHECK(outgoing.peek() == 'o');

  // Close an unused listener to get a refused port without TCP TIME_WAIT
  // interfering with the server that remains available for reconnection.
  WindowsServer unavailable(0);
  CHECK(unavailable.begin());
  uint16_t refusedPort = unavailable.port();
  unavailable.stop();
  CHECK(outgoing.connect(IPAddress(127, 0, 0, 1), refusedPort) == 0);
  CHECK(outgoing.getSocket() == INVALID_SOCKET);
  CHECK(!outgoing.connected());
  CHECK(!outgoing);
  CHECK(outgoing.available() == 0);
  CHECK(outgoing.read() == -1);
  waitReadable(second.getSocket());
  CHECK(!second.connected());
  second.stop();

  CHECK(outgoing.connect("127.0.0.1", server.port()) == 1);
  second = server.available(2000);
  CHECK(second);
  CHECK(outgoing.connect("not-an-ip", server.port()) == 0);
  CHECK(outgoing.getSocket() == INVALID_SOCKET);
  CHECK(!outgoing.connected());
  waitReadable(second.getSocket());
  CHECK(!second.connected());
}

void testServerOwnershipAndWaiting() {
  uint16_t port;
  {
    WindowsServer server(0);
    CHECK(server.begin());
    port = server.port();
    CHECK(server.begin());

    WindowsServer conflict(port);
    CHECK(!conflict.begin());
    CHECK(!conflict.available());

    Clock::time_point start = Clock::now();
    CHECK(!server.available());
    CHECK(elapsedMillis(start) < 500);
    start = Clock::now();
    CHECK(!server.available(100));
    CHECK(elapsedMillis(start) >= 60);
    CHECK(elapsedMillis(start) < 2000);

    server.stop();
    CHECK(conflict.begin());
  }
  WindowsServer replacement(port);
  CHECK(replacement.begin());

  // Keep the ownership/rebind assertions separate from accepted connections:
  // Windows exclusive binds can remain reserved while TCP is in TIME_WAIT.
  WindowsServer waiting(0);
  CHECK(waiting.begin());
  uint16_t waitingPort = waiting.port();
  Clock::time_point start = Clock::now();
  std::thread sender([waitingPort]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    WindowsClient client;
    CHECK(client.connect("127.0.0.1", waitingPort) == 1);
    CHECK(client.write(static_cast<uint8_t>('w')) == 1);
  });
  WindowsClient accepted = waiting.available(-1);
  sender.join();
  CHECK(accepted);
  CHECK(elapsedMillis(start) >= 40);
  CHECK(elapsedMillis(start) < 2000);
  waitReadable(accepted.getSocket());
  CHECK(accepted.read() == 'w');
  waitReadable(accepted.getSocket());
  CHECK(!accepted.connected());
}

std::string readResponse(WindowsClient& client) {
  std::string response;
  Clock::time_point start = Clock::now();
  uint8_t block[256];
  while (client.connected()) {
    int received = client.read(block, sizeof(block));
    if (received > 0) {
      response.append(reinterpret_cast<const char*>(block), received);
    } else {
      CHECK(elapsedMillis(start) < 2000);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  return response;
}

void testAppTimeout() {
  ClientPair pair;
  const char request[] = "GET / HTTP/1.1\r\nHost: localhost\r\n";
  CHECK(pair.peer.write(reinterpret_cast<const uint8_t*>(request),
                        sizeof(request) - 1) == sizeof(request) - 1);
  App app;
  app.setTimeout(60);
  Clock::time_point start = Clock::now();
  App::ProcessResult result = app.process(&pair.client);
  CHECK(elapsedMillis(start) >= 40);
  CHECK(elapsedMillis(start) < 2000);
  CHECK(!result.keepAlive && !result.responseOpen);
  CHECK(pair.client.getSocket() == INVALID_SOCKET);
  std::string response = readResponse(pair.peer);
  CHECK(response.find("HTTP/1.1 408 Request Timeout") != std::string::npos);
}

void rawBodyHandler(Request& request, Response& response) {
  // Header parsing can read ahead into the body. Raw Client access must retain
  // those bytes even when byte, peek and bulk reads are mixed.
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
  CHECK(pair.peer.write(reinterpret_cast<const uint8_t*>(request),
                        sizeof(request) - 1) == sizeof(request) - 1);
  waitQueued(pair.client.getSocket(), sizeof(request) - 1);
  App app;
  app.post("/", rawBodyHandler);
  app.process(&pair.client);
  pair.client.stop();
  std::string response = readResponse(pair.peer);
  CHECK(response.find("HTTP/1.1 200 OK") != std::string::npos);
  CHECK(response.find("raw body received") != std::string::npos);
}

}  // namespace

int main() {
  testAutomaticWinsockLifetime();
  // The raw TCP-pair helper needs Winsock before constructing wrapped clients.
  // CTest supplies an external timeout for a regression in connect or accept.
  WSADATA data;
  CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);
  testReadsAndTimeouts();
  testClientOwnership();
  testBufferedReads();
  testBufferedMovesAndStop();
  native_tests::testSharedClients<ClientPair>();
  testWritesAfterPeerClose();
  testWriteBackpressureTimeout();
  testConnectAndReconnect();
  testServerOwnershipAndWaiting();
  testAppTimeout();
  testAppRawBody();
  CHECK(WSACleanup() == 0);
  std::puts("Windows socket regression tests passed");
  return 0;
}
