#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <aWOT.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
  
WiFiServer server(80);
App app;
  
void indexHandler(Request &req, Response &res) {
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

  app.get("/", &indexHandler);
  server.begin();
}
  
void loop() {  
  WiFiClient client = server.available();
  
  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
