#include <WiFi.h>
#include <SPIFFS.h>
#include <aWOT.h>

#include "MimeTypes.h"

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

WiFiServer server(80);
Application app;

const char * indexFile = "/index.htm";

void fileServer(Request &req, Response &res) {
  if (req.method() != Request::GET) {
    return;
  }

  const char * path = req.path();

  if(strcmp(path, "/") == 0){
    path = indexFile;
  }

  if (!SPIFFS.exists(path)) {
    return;
  }

  File file = SPIFFS.open(path);

  if (file.isDirectory()) {
    return;
  }

  const char * mimeType = MimeTypes::getType(file.name());
  res.set("Content-Type", mimeType);

  while (file.available()) {
    res.write(file.read());
  }

  res.end();
}

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  
  File file = SPIFFS.open(indexFile, FILE_WRITE);
  file.print("<html><body><h1>Arduino File Server</h1></body></html>");
  file.close();
 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.use(&fileServer);

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
