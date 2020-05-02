#include <LoRa.h>
#include <WiFi.h>
#include <aWOT.h>

// Client: https://github.com/lasselukkari/ArduinoHttpStream/tree/master/examples/LoRaClient

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define SS 18
#define RST 14
#define DIO0 26
#define FREQUENCY 433E6

WiFiServer server(80);
Application app;
char body[16];
int counter;

void updateCounter(Request &req, Response &res) {
  req.readBytes(body, 13);
  counter = atoi(body);
}

void getCounter(Request &req, Response &res) { 
  res.print(counter); 
}

void setup() {
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  LoRa.setPins(SS, RST, DIO0);
  LoRa.begin(FREQUENCY);

  app.put("/counter", &updateCounter);
  app.get("/counter", &getCounter);

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }

  if (LoRa.parsePacket()) {
    app.process(&LoRa);
  }
}
