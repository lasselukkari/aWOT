#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <DNSServer.h>
#include <aWOT.h>

DNSServer dnsServer;
WiFiServer server(80);
Application app;
char redirectURL[30];

void redirect(Request &req, Response &res) {
  if (!res.statusSent()) {
    res.set("Location", redirectURL);
    res.sendStatus(302);
  }
}

void index(Request &req, Response &res) {
  res.print("Captive portal index");
}

void popup(Request &req, Response &res) {
  res.print("Captive portal popup");
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP("TestNetwork", "TestNetwork");
  IPAddress ip = WiFi.softAPIP();
  sprintf(redirectURL, "http://%d.%d.%d.%d/popup", ip[0], ip[1], ip[2], ip[3]);
  Serial.println(ip);

  app.get("/", &index);
  app.get("/popup", &popup);
  app.use(&redirect);

  server.begin();

  dnsServer.start(53, "*", ip);
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
    client.stop();
  }

  dnsServer.processNextRequest();
}
