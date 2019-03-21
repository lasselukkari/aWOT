---
layout: page
title: Using JSON in aWOT apps
menu: guide
lang: en
redirect_from: "/guide/json.html"
---
# Working with JSON data

The [Request](/{{ page.lang }}/1x/api.html#req) and [Response](/{{ page.lang }}/1x/api.html#res) classes both implement the [Arduino Stream interface](https://www.arduino.cc/reference/en/language/functions/communication/stream/). This makes it possible to directly forward them to Arduino JSON libraries for parsing and serializing. 

The example uses the [aJSON](https://github.com/interactive-matter/aJson/) library. The app has two routes: one or reading the JSON encoded data and one for updating it. See the aJSON documentation for details how to access and manipulate the properties individually.

```arduino
#include <WiFi.h>
#include <aJSON.h>
#include "aWOT.h"

WiFiServer server(80);
Application app;
aJsonObject* example = aJson.parse("{\"example\":\"data\"}");

void readExample(Request &req, Response &res) {
  res.set("Content-Type", "application/json");
  aJsonStream stream(&res);
  aJson.print(example, &stream);
}

void updateExample(Request &req, Response &res) {
  aJsonStream stream(&req);
  example = aJson.parse(&stream);
  return readExample(req, res);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin("network", "password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.get("/example", &readExample);
  app.put("/example", &updateExample);

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }
}
```

You can test the api with curl. To read the current data:

```sh
$ curl 'http://192.168.1.140/example' -v
*   Trying 192.168.1.140...
* TCP_NODELAY set
* Connected to 192.168.1.140 (192.168.1.140) port 80 (#0)
> GET /example HTTP/1.1
> Host: 192.168.1.140
> User-Agent: curl/7.54.0
> Accept: */*
> 
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Type: application/json
< 
* Closing connection 0
{"example":"data"}
```

And to update:

```sh
$ curl 'http://192.168.1.140/example' -X PUT -H "Content-Type: application/json" -d '{"new":"data"}' -v
*   Trying 192.168.1.140...
* TCP_NODELAY set
* Connected to 192.168.1.140 (192.168.1.140) port 80 (#0)
> PUT /example HTTP/1.1
> Host: 192.168.1.140
> User-Agent: curl/7.54.0
> Accept: */*
> Content-Type: application/json
> Content-Length: 14
> 
* upload completely sent off: 14 out of 14 bytes
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Type: application/json
< 
* Closing connection 0
{"new":"data"}
```
