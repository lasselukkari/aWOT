#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <aWOT.h>
#include <SseChannel.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

WiFiServer server(80);
App app;
SseChannel<WiFiClient, 4> events;
unsigned long lastPush = 0;

void indexHandler(Request& req, Response& res) {
  P(page) =
      "<!doctype html><title>SSE</title><p>Uptime: <span id='uptime'></span></p>"
      "<script>new EventSource('/events').onmessage=e=>"
      "document.getElementById('uptime').textContent=e.data+' seconds';</script>";
  res.set("Content-Type", "text/html");
  res.printP(page);
}

void sseHandler(Request& req, Response& res) { events.open(res); }

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());
  app.get("/", indexHandler);
  app.get("/events", sseHandler);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client.connected()) {
    App::ProcessResult result = app.process(&client);
    if (result.responseOpen) {
      events.add(client);
    } else {
      client.stop();
    }
  }

  unsigned long now = millis();
  if (now - lastPush >= 1000) {
    lastPush = now;
    char uptime[16];
    snprintf(uptime, sizeof(uptime), "%lu", now / 1000);
    events.send(uptime);
  }
}
