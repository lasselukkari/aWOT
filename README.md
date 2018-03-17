aWOT
==========

Arduino web server library.

## Tested with
* Arduino Uno
* Arduino Leonardo
* Arduino Mega 2560
* Arduino Due
* Teensy 3
* Generic ESP8266
* NodeMCU 1.0
* Adafruit Feather M0 + ATWINC1500
* ESP32

## Usage

### Full example using Arduino Ehternet
```cpp
#include <SPI.h>
#include <Ethernet.h>
#include <aWOT.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(80);
WebApp app;

// define a handler function
void indexCmd(Request &req, Response &res) {

  // P macro for printing strings from program memory
  P(index) =
    "<html>\n"
    "<head>\n"
    "<title>Hello World!</title>\n"
    "</head>\n"
    "<body>\n"
    "<h1>Greetings middle earth!</h1>\n"
    "</body>\n"
    "</html>";

  res.success("text/html");
  res.printP(index);
}

void setup() {
  Serial.begin(115200);

  if (Ethernet.begin(mac)) {
    Serial.println(Ethernet.localIP());
  } else{
    Serial.println("Ethernet failed");
  }

  // mount the handler to the default router
  app.get("/", &indexCmd);
}

void loop(){
  EthernetClient client = server.available();
  if (client.connected()) {
      app.process(&client);
  }
}
```

### Query parameters
```cpp
// HTTP GET /cats?type=lolcat
void queryParams(Request &req, Response &res) {
  char type[64];
  req.query("type", type, 64);

  res.success("text/plain");
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
  char * catId = req.route("catId"); // "lolcat"

  res.success("text/plain");
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

  res.success("text/plain");
  while (req.contentLeft()) {
    req.postParam(name, 10, value, 64);
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
  char * userAgent = req.header("User-Agent"); // "Mozilla/5.0 (Macintosh; Inte ...."

  res.set("Cookie", "lolcat"); // will set Cookie header value to "lolcat"
  res.success("text/plain");
  res.print(userAgent);
}

void setup() {
  // other setup

  app.readHeader("User-Agent", userAgentBuffer, 200);
  app.get("/useragent", &headers);
}
```

### Routers
```cpp
WebApp app;
Router cats("/cats");

void looooong(Request &req, Response &res) {
  res.success("text/html");
  res.print("looooong cat is long!");
}

void ceiling(Request &req, Response &res) {
  res.success("text/html");
  res.print("ceiling cat is watching you debug!");
}

void nyannyan(Request &req, Response &res) {
  res.success("text/html");

  for (int i = 0; i < 100; i++) {
      res.print("nyan ");
  }
}

void setup() {
  // other setup

  cats.get("/long", &looooong);
  cats.get("/ceiling", &ceiling);
  cats.get("/nyan", &nyannyan);

  app.use(&cats);
}
```

## Acknowledgements
Based on Webduino, Copyright 2009-2014 Ben Combee, Ran Talbott, Christopher Lee, Martin Lormes, Francisco M Cuenca-Acuna

## Licence
MIT
