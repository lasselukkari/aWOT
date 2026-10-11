/*
  HelloWorld.cpp - aWOT example using native Linux POSIX sockets

  Run ./HelloWorld, then visit http://localhost:8080/.
*/

#include <aWOT.h>

#include <cstdlib>
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

int main() {
  Serial.begin(115200);
  app.get("/", &indexCmd);
  LinuxServer server(8080);
  if (!server.begin()) return EXIT_FAILURE;
  Serial.println("Server listening on port 8080 (multithreaded)");
  Serial.flush();

  for (;;) {
    // Wait in poll until a connection arrives, with no idle polling loop.
    LinuxClient client = server.available(-1);
    if (client.connected()) {
      std::thread(handleClient, std::move(client)).detach();
    }
  }
}
