#include <WiFi.h>
#include <aWOT.h>

#define MAX_CLIENTS 4
#define MAX_IDLE 5000UL
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

WiFiServer server(80);
WiFiClient clients[MAX_CLIENTS];
App app;
unsigned long lastActivity[MAX_CLIENTS] = {};

void keepAlive(Request& req, Response& res) {
  res.set("Connection", "keep-alive");
  req.next();
}

void indexHandler(Request& req, Response& res) { res.print("Hello World!"); }

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());
  app.use(keepAlive);
  app.get("/", indexHandler);
  server.begin();
}

void loop() {
  unsigned long now = millis();
  WiFiClient incoming = server.available();
  if (incoming.connected()) {
    bool stored = false;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
      if (!clients[i].connected()) {
        clients[i] = incoming;
        lastActivity[i] = now;
        stored = true;
        break;
      }
    }
    if (!stored) incoming.stop();
  }

  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i].available()) {
      App::ProcessResult result = app.process(&clients[i]);
      if (!result.keepAlive) clients[i].stop();
      lastActivity[i] = millis();
    } else if (clients[i].connected() &&
               now - lastActivity[i] >= MAX_IDLE) {
      clients[i].stop();
    }
  }
}
