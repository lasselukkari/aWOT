#include <ETH.h>
#include <aWOT.h>

NetworkServer server(80);
App app;

void indexHandler(Request &req, Response &res) {
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

  app.get("/", &indexHandler);
  server.begin();
}

void loop() {
  NetworkClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }
}
