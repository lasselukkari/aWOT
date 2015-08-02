aWOT
==========

A web of things. Arduino family compatible web application framework.

## Documentation
http://www.awot.net

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
  if (client){
      app.process(&client);
  }
}
```

### Query parameters
```cpp
// HTTP GET /cats?type=lolcat
void queryParams(Request &req, Response &res) {
  char * type = req.query("type"); // "lolcat"

  res.success("text/plain");
  res.print(type);
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

  // other setup ...

  app.get("/cats/:catId", &routeParams);
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

## Related projects 

* A more advanced example created with [AngularJS Arduino Generator]( https://github.com/lasselukkari/generator-angular-arduino) can be found [here](https://github.com/lasselukkari/MaterialSwitch).

* To generate static asset file payloads use [grunt-awot](https://github.com/lasselukkari/grunt-awot)

* For uploading Arduino projects you can use [grunt-arduino](https://github.com/lasselukkari/grunt-arduino)

## Licence

Copyright (c) 2014, Lasse Lukkari 

Based on 

Webduino,
Copyright 2009-2014 Ben Combee, Ran Talbott, Christopher Lee, Martin Lormes
Francisco M Cuenca-Acuna

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
