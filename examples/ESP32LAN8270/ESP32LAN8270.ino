#include <ETH.h>
#include <aWOT.h>

NetworkServer server(80); // ESP32 core ETH Library uses NetworkServer instead of WiFiServer
Application app;

void index(Request &req, Response &res) {
  res.print("Hello World!");
}

void setup() {
  Serial.begin(115200);

  ETH.begin();
  while (!ETH.localIP()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(ETH.localIP());

  app.get("/", &index);
  server.begin();
}

void loop() {
  NetworkClient client = server.available(); // ESP32 core ETH Library uses NetworkClient instead of WiFiClient

  if (client.connected()) {
    app.process(&client);
  }
}
