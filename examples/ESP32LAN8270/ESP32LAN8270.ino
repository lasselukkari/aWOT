#include <ETH.h>
#include <aWOT.h>

WiFiServer server(80);
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
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }
}
