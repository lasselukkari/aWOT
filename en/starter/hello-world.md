---
layout: page
title: aWOT "Hello World" example
menu: starter
lang: en
---

# Hello world example

Install **aWOT 4.0.0** in Arduino IDE: open **Tools → Manage Libraries**, search
for **aWOT**, select version **4.0.0**, and click **Install**. Choose your board
and port before uploading. The examples on this site use `App`, the v4
application class. Existing applications can follow the
[migration guide](/en/guide/migrating-to-v4.html).

<h2 id="hello-example">Arduino Uno with Ethernet</h2>

Connect an Ethernet shield and a network cable to an Arduino Uno. Install
**Ethernet 2.0.2** through Library Manager. This complete sketch requests an
address using DHCP and serves `Hello World!` at `/`:

```arduino
#include <SPI.h>
#include <Ethernet.h>
#include <aWOT.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetServer server(80);
App app;

void hello(Request& req, Response& res) {
  P(message) = "Hello World!";
  res.printP(message);
}

void setup() {
  Serial.begin(115200);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP failed; check the Ethernet connection.");
    while (true) delay(1000);
  }
  Serial.println(Ethernet.localIP());
  app.get("/", hello);
  server.begin();
}

void loop() {
  Ethernet.maintain();
  EthernetClient client = server.available();
  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
```

Use a MAC address unique on your network. Open the Serial Monitor at **115200
baud**, then visit the printed IP address, for example `http://192.168.1.140/`.
Other paths receive `404 Not Found`.

`P(message)` stores the string in program memory and `res.printP(message)`
prints it. The Uno uses small default HTTP buffers; see
[configuration](/en/guide/configuration.html) for their sizes and limits.

`app.process(&client)` handles one HTTP request and finalizes its response.
The sketch owns the transport and calls `client.stop()` afterward. This example
closes every connection; retaining one requires checking the
[processing result](/en/guide/connections.html).

<h2 id="wifi">ESP32 with Wi-Fi</h2>

Install the ESP32 Arduino board package and select your ESP32 board. This
complete variant uses the same handlers and connection lifecycle. Set the
network credentials before uploading.

```arduino
#include <WiFi.h>
#include <aWOT.h>

WiFiServer server(80);
App app;

void hello(Request& req, Response& res) {
  res.print("Hello World!");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());
  app.get("/", hello);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
```

These sketches are checked with Arduino AVR core **1.8.8** on Uno and ESP32
core **3.3.10** on `nodemcu-32s`. Continue with
[basic routing](/en/starter/basic-routing.html).
