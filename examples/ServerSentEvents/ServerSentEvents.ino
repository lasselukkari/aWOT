
#include <WiFi.h>
#include <aWOT.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

WiFiServer server(80);
Application app;
Stream* sseClient = NULL;
unsigned long timeToPush = 0;

void sseHandler(Request& req, Response& res) {
  res.set("Content-Type", "text/event-stream");
  res.set("Cache-Control", "no-cache");
  res.set("Connection", "keep-alive");

  // Headers are sent automatically when buffer flushes or when we flush
  // manually
  res.flush();

  // Prevent automatic stream closure
  res.keepOpen();

  // Save the connection for the main loop
  sseClient = req.stream();
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.get("/events", &sseHandler);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }

  // Push data every second if we have a connected SSE client
  if (sseClient && millis() > timeToPush) {
    timeToPush = millis() + 1000;

    // We are now writing 'raw' to the stream which is in chunked mode.
    // We must format every write as a chunk: <LengthHex>\r\n<Data>\r\n

    String message = "data: " + String(millis()) + "\n\n";

    sseClient->print(message.length(), HEX);
    sseClient->print("\r\n");
    sseClient->print(message);
    sseClient->print("\r\n");
  }
}
