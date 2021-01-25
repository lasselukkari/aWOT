aWOT [![Build Status](https://travis-ci.org/lasselukkari/aWOT.svg?branch=master)](https://travis-ci.org/lasselukkari/aWOT)
==========

Arduino web server library.

## Documentation
#### 1. Getting started
 * [Hello World](https://awot.net/en/starter/hello-world.html)
 * [Basic routing](https://awot.net/en/starter/basic-routing.html)
 * [Application generator](https://awot.net/en/starter/generator.html)
 * [Serving static files](https://awot.net/en/starter/static-files.html)
#### 2. Guide
 * [Routing](https://awot.net/en/guide/routing.html)
 * [Middleware](https://awot.net/en/guide/middleware.html)
 * [Working with JSON data](https://awot.net/en/guide/json.html)
 * [Tutorial: Deploying a React app on the ESP32](https://awot.net/en/guide/tutorial.html)
#### 3. API Reference
 * [Application](https://awot.net/en/3x/api.html#app)
 * [Request](https://awot.net/en/3x/api.html#req)
 * [Response](https://awot.net/en/3x/api.html#res)
 * [Router](https://awot.net/en/3x/api.html#router)

## Compatibility

The aWOT web server library has been designed to work with all Arduino compatible development boards and networking options. This also means that switching the board or changing from WiFi to Ethernet will require minimal changes. The examples directory shows you how to use the library with the most popular Ethernet and WiFi libraries 

However there are few subtle differences that need to be taken into account. Also unfortunately some of the WiFi and Ethernet libraries have bugs that prevent the library from working properly.

Previously the library has been tested with:
- Adafruit Feather M0 WiFi
- Arduino UNO + WIZnet W5100
- Arduino Mega + WIZnet W5100
- Arduino Due + WIZnet W5100
- Arduino MKR WiFi 1010 (see notes)
- Teensy 3.0 + WIZnet W5200
- Teensy 4.1 Ethernet (see notes)
- ESP32 + Wiznet W5500 Ethernet
- ESP32 + LAN8270 Ethernet
- ESP32 WiFi
- ESP8266 WiFi

#### ESP32 and ESP8266 WiFi

In both of the ESP Arduino cores the WiFiClient closes the connection automatically in the class destructor. This means that the client.stop(), does not need to be explicitly called but you will need to take extra steps if you want to keep the connection alive.

#### ESP32 + Wiznet W5500
The current version of the ESP32 Arduino core uses a non standard version of the Server class. Until the ESP32 core is fixed you need to manually modify the begin function in the Server.h if you want to use the Ethernet library that is shipped with the core.

Bug report: https://github.com/espressif/arduino-esp32/issues/2704

#### WifiNina
The WifiNina firmware currently has a bug that causes large writes with the client to fail randomly. The fix is already implemented but it has not yet been merged to the master or released.

Bug report: https://github.com/arduino/nina-fw/issues/61

#### Teensy 4.1 + Ethernet
The Teensy 4.1 Ethernet library currently has a bug that causes the connection to stall and reset when connections to the server are opened in fast phase. The bug has been verified but not fixed yet.

Bug report: https://github.com/vjmuzik/NativeEthernet/issues/7

#### Arduino UNO
Because of the limited RAM and ROM Arduino UNO is on the edge of being usable for anything more complicated. If you want to use this library together with the SD card or any JSON parsing library, pay attention that you do not run out of memory.

## Examples
### Hello World
```cpp
#include <WiFi.h>
#include <aWOT.h>

WiFiServer server(80);
Application app;

void index(Request &req, Response &res) {
  res.print("Hello World!");
}

void setup() {
  Serial.begin(115200);

  WiFi.begin("ssid", "password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.get("/", &index);
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

### Query parameters
```cpp
// HTTP GET /cats?type=lolcat
void queryParams(Request &req, Response &res) {
  char type[64];
  req.query("type", type, 64);

  res.print(type); // "lolcat"
}

void setup() {
  // other setup ...
  app.get("/cats", &queryParams);
}
```

### Route paramaters
```cpp
// HTTP GET /cats/lolcat
void routeParams(Request &req, Response &res) {
  char catId[64];
  req.route("catId", catId, 64);

  res.print(catId);
}

void setup() {
  // other setup
  app.get("/cats/:catId", &routeParams);
}
```

### Post parameters
```cpp
void postParams(Request &req, Response &res) {
  char name[10];
  char value[64];

  while (req.left()) {
    req.form(name, 10, value, 64);
    res.print(name);
    res.print(": ");
    res.println(value);
  }
}

void setup() {
  // other setup

  app.post("/form", &postParams);
}
```

### Reading and writing headers
```cpp
char userAgentBuffer[200];

// HTTP GET /headers
void headers(Request &req, Response &res) {
  char * userAgent = req.get("User-Agent"); // "Mozilla/5.0 (Macintosh; Inte ...."

  res.set("Cookie", "lolcat"); // will set Cookie header value to "lolcat"
  res.print(userAgent);
}

void setup() {
  // other setup

  // header names are handled case insensitive
  app.header("User-Agent", userAgentBuffer, 200); 
  app.get("/useragent", &headers);
}
```

### Routers
```cpp
Application app;
Router cats;

void looooong(Request &req, Response &res) {
  res.print("looooong cat is long!");
}

void ceiling(Request &req, Response &res) {
  res.print("ceiling cat is watching you debug!");
}

void nyannyan(Request &req, Response &res) {
  for (int i = 0; i < 100; i++) {
      res.print("nyan ");
  }
}

void setup() {
  // other setup

  cats.get("/long", &looooong);
  cats.get("/ceiling", &ceiling);
  cats.get("/nyan", &nyannyan);

  app.use("/cats", &cats);
}
```

## Acknowledgements
Based on Webduino, Copyright 2009-2014 Ben Combee, Ran Talbott, Christopher Lee, Martin Lormes, Francisco M Cuenca-Acuna

## Licence
MIT
