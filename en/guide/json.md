---
layout: page
title: Using JSON in aWOT apps
menu: guide
lang: en
redirect_from: "/guide/json.html"
---
# Working with JSON data

The [Request](/{{ page.lang }}/3x/api.html#req) inherits from the Arduino [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/) base class. This makes it possible to directly forward the request to Arduino JSON libraries for parsing and serializing. 

This example uses the [aJSON](https://github.com/interactive-matter/aJson/) library but [ArduinoJson](https://arduinojson.org/) can [deserialize streams](https://arduinojson.org/v6/api/json/deserializejson/) as well. 

The app has two routes: one or reading the JSON encoded data and one for updating it.

```arduino
#include <WiFi.h>
#include <aJSON.h>
#include "aWOT.h"

WiFiServer server(80);
Application app;

char* userFilter[3] = {"name", "password", NULL};
aJsonObject* user = aJson.parse("{\"name\":\"John Smith\",\"password\":\"secret\"}");

void readUser(Request & req, Response & res) {
  res.set("Content-Type", "application/json");
  aJsonStream stream(&req);
  aJson.print(user, &stream);
}

void updateUser(Request & req, Response & res) {
  aJsonStream stream(&req);

  aJsonObject* newUser = aJson.parse(&stream, userFilter);
  if (!newUser) {
    return res.sendStatus(400);
  }

  aJsonObject* name = aJson.getObjectItem(newUser, "name");
  if (!name || name->type != aJson_String) {
    return res.sendStatus(400);
  }

  aJsonObject* password = aJson.getObjectItem(newUser, "password");
  if (!password || password->type != aJson_String) {
    return res.sendStatus(400);
  }

  aJson.deleteItem(user);
  user = newUser;

  return readUser(req, res);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin("network", "password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.get("/user", &readUser);
  app.put("/user", &updateUser);

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
$ curl 'http://192.168.1.140/user' -v
*   Trying 192.168.1.140...
* TCP_NODELAY set
* Connected to 192.168.1.140 (192.168.1.140) port 80 (#0)
> GET /user HTTP/1.1
> Host: 192.168.1.140
> User-Agent: curl/7.54.0
> Accept: */*
> 
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Type: application/json
< 
* Closing connection 0
{"name":"John Smith","password":"secret"}
```

And to update:

```sh
$ curl 'http://192.168.1.140/user' -X PUT -H "Content-Type: application/json" -d '{"name":"4N0NYM0U5", "password":"pwn3d"}' -v
*   Trying 192.168.1.140...
* TCP_NODELAY set
* Connected to 192.168.1.140 (192.168.1.140) port 80 (#0)
> PUT /user HTTP/1.1
> Host: 192.168.1.140
> User-Agent: curl/7.54.0
> Accept: */*
> Content-Type: application/json
> Content-Length: 40
> 
* upload completely sent off: 40 out of 40 bytes
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Type: application/json
< 
* Closing connection 0
{"name":"4N0NYM0U5","password":"pwn3d"}
```
