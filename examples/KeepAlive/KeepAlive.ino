#include <WiFi.h>
#include <aWOT.h>

#define MAX_CLIENTS 16
#define MAX_IDLE 5000
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

WiFiServer server(80);
WiFiClient clients[MAX_CLIENTS];
Application app;
unsigned long ttls[MAX_CLIENTS] = {};

void keepAlive(Request &req, Response &res) {
  res.set("Connection", "keep-alive");
}

void index(Request &req, Response &res) {
  res.print("Hello World!");
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.use(&keepAlive);
  app.get("/", &index);
  server.begin();
}

void loop() {
  unsigned long now = millis();

  if (server.hasClient()) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (!clients[i].connected()) {
        clients[i] = server.available();
        ttls[i] = now +  MAX_IDLE;
        break;
      }
    }
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].available()) {
      app.process(&clients[i]);
      ttls[i] = now +  MAX_IDLE;
    } else if (ttls[i] && ttls[i] < now) {
      clients[i].stop();
      ttls[i] = 0;
    }
  }
}
