/*
  HelloWorld.cpp - aWOT example using native Windows Winsock sockets

  Run HelloWorld.exe, then visit http://localhost:8080/.
*/

#include <aWOT.h>

#include <cstdlib>
#include <thread>
#include <utility>

#include "WindowsSocket.h"

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

void handleClient(WindowsClient client) {
  app.process(&client);
  client.stop();
}

int main() {
  Serial.begin(115200);
  app.get("/", &indexCmd);
  WindowsServer server(8080);
  if (!server.begin()) return EXIT_FAILURE;
  Serial.println("Server listening on port 8080 (multithreaded)");
  Serial.flush();

  for (;;) {
    // Wait in select until a connection arrives, with no idle polling loop.
    WindowsClient client = server.available(-1);
    if (client.connected()) {
      std::thread(handleClient, std::move(client)).detach();
    }
  }
}
