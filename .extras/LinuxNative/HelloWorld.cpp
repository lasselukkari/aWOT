/*
  HelloWorld.cpp - aWOT example using native Linux POSIX sockets

  Run ./HelloWorld, then visit http://localhost:8080/.
  Use --port PORT to select a different port, or --port 0 for an ephemeral port.
*/

#include <aWOT.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <utility>

#include "LinuxSocket.h"

App app;

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

void handleClient(LinuxClient client) {
  app.process(&client);
  client.stop();
}

bool parsePort(int argc, char* argv[], uint16_t& port) {
  if (argc == 1) return true;
  if (argc != 3 || std::strcmp(argv[1], "--port") != 0 || !argv[2][0]) {
    return false;
  }
  unsigned value = 0;
  for (const char* digit = argv[2]; *digit; ++digit) {
    if (*digit < '0' || *digit > '9') return false;
    value = value * 10 + static_cast<unsigned>(*digit - '0');
    if (value > 65535) return false;
  }
  port = static_cast<uint16_t>(value);
  return true;
}

int main(int argc, char* argv[]) {
  uint16_t port = 8080;
  if (!parsePort(argc, argv, port)) {
    std::fprintf(stderr, "Usage: HelloWorld [--port 0..65535]\n");
    return EXIT_FAILURE;
  }

  Serial.begin(115200);
  app.get("/", &indexCmd);
  LinuxServer server(port);
  if (!server.begin()) return EXIT_FAILURE;
  Serial.print("Server listening on port ");
  Serial.print(server.port());
  Serial.println(" (multithreaded)");
  Serial.flush();

  for (;;) {
    // Wait in poll until a connection arrives, with no idle polling loop.
    LinuxClient client = server.available(-1);
    if (client.connected()) {
      std::thread(handleClient, std::move(client)).detach();
    }
  }
}
