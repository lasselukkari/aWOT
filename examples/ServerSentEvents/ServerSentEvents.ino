
#include <WiFi.h>
#include <aWOT.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define MAX_SSE_CLIENTS 4

WiFiServer server(80);
App app;
WiFiClient sseClients[MAX_SSE_CLIENTS];
unsigned long timeToPush = 0;

const char indexPage[] PROGMEM = R"(
<!DOCTYPE html>
<html>
<head><title>SSE Demo</title></head>
<body>
  <h1>Connected clients: <span id="count">0</span></h1>
  <script>
    const events = new EventSource('/events');
    events.onmessage = e => {
      document.getElementById('count').textContent = e.data;
    };
  </script>
</body>
</html>
)";

void indexHandler(Request& req, Response& res) {
  res.set("Content-Type", "text/html");
  res.printP(reinterpret_cast<const unsigned char*>(indexPage));
}

void sseHandler(Request& req, Response& res) {
  res.set("Content-Type", "text/event-stream");
  res.set("Cache-Control", "no-cache");
  res.set("Connection", "keep-alive");
  res.flush();
  res.keepOpen();
}

int countClients() {
  int count = 0;
  for (int i = 0; i < MAX_SSE_CLIENTS; i++) {
    if (sseClients[i].connected()) count++;
  }
  return count;
}

void sendToAllClients(const String& message) {
  for (int i = 0; i < MAX_SSE_CLIENTS; i++) {
    if (sseClients[i].connected()) {
      sseClients[i].print(message.length(), HEX);
      sseClients[i].print("\r\n");
      sseClients[i].print(message);
      sseClients[i].print("\r\n");
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.get("/", &indexHandler);
  app.get("/events", &sseHandler);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    App::ProcessResult result = app.process(&client);

    // If this was a streaming request, store the client for later
    if (result.responseOpen) {
      for (int i = 0; i < MAX_SSE_CLIENTS; i++) {
        if (!sseClients[i].connected()) {
          sseClients[i] = client;
          break;
        }
      }
    }
  }

  if (millis() > timeToPush) {
    timeToPush = millis() + 1000;
    String message = "data: " + String(countClients()) + "\n\n";
    sendToAllClients(message);
  }
}
