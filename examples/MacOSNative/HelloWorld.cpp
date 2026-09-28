/*
  HelloWorld.cpp - aWOT example using native macOS BSD sockets

  This example demonstrates using the aWOT library on macOS without
  Arduino hardware. Uses multithreading for concurrent request handling.

  Build: make
  Run:   ./HelloWorld (then visit http://localhost:8080/)
*/

#include <aWOT.h>

#include <atomic>
#include <thread>

#include "MacOSSocket.h"

MacOSServer server(8080);
App app;
std::atomic<int> activeConnections(0);

void indexCmd(Request&, Response& res) {
  res.set("Content-Type", "text/html");
  res.print("<html>\n");
  res.print("<head>\n");
  res.print("<title>Hello World!</title>\n");
  res.print("</head>\n");
  res.print("<body>\n");
  res.print("<h1>Greetings middle earth!</h1>\n");
  res.print("</body>\n");
  res.print("</html>");
}

// Handle a single client - one request and close
void handleClient(MacOSClient client) {
  activeConnections++;
  app.process(&client);
  client.stop();
  activeConnections--;
}

bool startServer() {
  Serial.begin(115200);

  app.get("/", &indexCmd);
  if (!server.begin()) return false;

  Serial.println("Server listening on port 8080 (multithreaded)");
  Serial.flush();
  return true;
}

void loop() {
  // Sleep in poll until a connection arrives instead of spinning while idle.
  MacOSClient client = server.available(-1);
  if (client.connected()) {
    // Spawn a new thread to handle this client
    std::thread(handleClient, std::move(client)).detach();
  }
}

int main() {
  if (!startServer()) return EXIT_FAILURE;
  while (true) {
    loop();
  }
  return 0;
}
