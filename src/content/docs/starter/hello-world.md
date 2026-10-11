---
title: "aWOT \"Hello World\" example"
description: "Run your first aWOT web server on an ESP32 or an Arduino with Ethernet."
route: "/en/starter/hello-world.html"
section: "starter"
---

# Hello world example

Install **aWOT 4.0.0** in Arduino IDE: open **Tools → Manage Libraries**, search
for **aWOT**, select version **4.0.0**, and click **Install**. Choose your board
and port before uploading. The examples on this site use `App`, the v4
application class. Existing applications can follow the
[migration guide](/en/guide/migrating-to-v4.html).

You can also run aWOT on your Mac without a board. Follow the
[native macOS guide](/en/guide/macos.html) to build a server for localhost.

Create a new sketch in the Arduino IDE and save it in a directory with the same
name, such as `WebServer/WebServer.ino`. Copy the sketch below for your board,
set the network details, and upload it.

<h2 id="wifi">ESP32 with Wi-Fi</h2>


Install the ESP32 Arduino board package and select your ESP32 board. This
complete sketch serves `Hello World!` at `/`. Set the network credentials
before uploading.

```cpp
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

Open the Serial Monitor at **115200 baud**, then visit the printed IP address,
for example `http://192.168.1.140/`.
Other paths receive `404 Not Found`.

`app.process(&client)` handles one HTTP request and finalizes its response.
Both sketches own the transport and call `client.stop()` afterward. They
close every connection; retaining one requires checking the
[processing result](/en/guide/connections.html).

<h2 id="hello-example">Arduino Uno with Ethernet</h2>


Connect an Ethernet shield and a network cable to an Arduino Uno. Install
**Ethernet 2.0.2** through Library Manager. This complete variant requests an
address using DHCP and serves the same response:

```cpp
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

Use a MAC address unique on your network. `P(message)` stores the string in
program memory and `res.printP(message)` prints it. The Uno uses small default
HTTP buffers; see [configuration](/en/guide/configuration.html) for their sizes
and limits.

These sketches are checked with ESP32 core **3.3.10** on `nodemcu-32s` and
Arduino AVR core **1.8.8** on Uno. Continue with
[basic routing](/en/starter/basic-routing.html).
