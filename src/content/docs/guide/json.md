---
title: "Using JSON in aWOT apps"
description: "Read and validate JSON requests and send JSON responses with ArduinoJson and aWOT."
route: "/en/guide/json.html"
section: "guide"
---

# Working with JSON data

`Request` inherits Arduino `Stream` and `Response` inherits `Print`. Read JSON
with `deserializeJson(doc, req)` and write it with `serializeJson(doc, res)`;
the request and response have different roles.

Install **ArduinoJson 7.4.3** through Arduino Library Manager, alongside
[aWOT 4.0.0](/en/starter/hello-world.html). The
[ArduinoJson installation guide](https://arduinojson.org/v7/how-to/install-arduinojson/)
describes version selection. Its official API documents
[Stream input](https://arduinojson.org/v7/api/json/deserializejson/) and
[Print output](https://arduinojson.org/v7/api/json/serializejson/).

<h2 id="example">Read and update a user</h2>


This complete ESP32 sketch returns a user name at `GET /user` and updates it at
`PUT /user`. It requires a known, nonzero Content-Length of at most 256 bytes,
checks the parser result, requires a string name of at most 32 bytes, and
rejects non-whitespace bytes after the JSON object. Set the Wi-Fi credentials
before uploading.

```cpp
#include <WiFi.h>
#include <ArduinoJson.h>
#include <aWOT.h>
#include <ctype.h>
#include <string.h>

WiFiServer server(80);
App app;
char userName[33] = "John Smith";

void readUser(Request& req, Response& res) {
  JsonDocument doc;
  doc["name"] = userName;
  if (doc.overflowed()) {
    res.sendStatus(500);
    return;
  }
  res.set("Content-Type", "application/json");
  serializeJson(doc, res);
  res.flush();
  if (res.getWriteError()) Serial.println("JSON response write failed");
}

void updateUser(Request& req, Response& res) {
  const char* type = req.get("Content-Type");
  if (!type || strcmp(type, "application/json") != 0) {
    res.sendStatus(415);
    return;
  }
  if (req.left() <= 0 || req.left() > 256) {
    res.sendStatus(400);
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(
      doc, req, DeserializationOption::NestingLimit(4));
  if (error) {
    Serial.println(error.c_str());
    res.sendStatus(400);
    return;
  }
  if (!doc["name"].is<const char*>()) {
    res.sendStatus(400);
    return;
  }
  const char* name = doc["name"].as<const char*>();
  if (strlen(name) == 0 || strlen(name) >= sizeof(userName)) {
    res.sendStatus(400);
    return;
  }

  // The parser can stop at the closing brace; consume trailing whitespace.
  while (req.left() > 0) {
    char ch;
    if (req.readBytes(&ch, 1) != 1 || !isspace((unsigned char)ch)) {
      res.sendStatus(400);
      return;
    }
  }
  strcpy(userName, name);
  readUser(req, res);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());
  app.get("/user", readUser);
  app.put("/user", updateUser);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client.connected()) {
    char contentType[48];
    Request::Header headers[] = { {"Content-Type", contentType} };
    app.process(&client, headers);
    client.stop();
  }
}
```

The example accepts exactly `Content-Type: application/json`. Extend that
check if your clients send media type parameters. It reads the body directly
from `req`, while the document keeps the parsed values in memory. ArduinoJson
7 allocates document storage on the heap; the 256-byte body limit is an input
limit, rather than a promise about heap usage. Choose limits for your board.

The parser returns a
[DeserializationError](https://arduinojson.org/v7/api/misc/deserializationerror/)
on invalid or incomplete input. The
[type check](https://arduinojson.org/v7/api/jsonvariant/is/)
prevents a missing or non-string name from reaching `strcpy()`. The response
document checks
[allocation failure](https://arduinojson.org/v7/api/jsondocument/overflowed/)
before sending headers.

`res.flush()` makes buffered output reach the transport before checking
`getWriteError()`. Bytes accepted by `serializeJson()` are not a guarantee of
delivery to the browser. See [connections](/en/guide/connections.html#writes).

<h2 id="test-with-curl">Test with curl</h2>


Replace the IP address with the one printed by your board:

```bash
curl --silent http://192.168.1.140/user
```

```json
{"name":"John Smith"}
```

```bash
curl --silent --request PUT http://192.168.1.140/user \
  --header 'Content-Type: application/json' \
  --data '{"name":"Arduino"}'
```

```json
{"name":"Arduino"}
```

A body such as `{"name":42}` receives `400 Bad Request`; omitting the
Content-Type header receives `415 Unsupported Media Type`. Send a body with
Content-Length: this sketch rejects chunked bodies because their total length
is unknown, although aWOT supports chunked HTTP/1.1 requests. See
[request body reads](/en/guide/request-data.html#body) for the decoding contract.
This sketch closes each connection after the response, so unread bytes on a
rejected request cannot become a subsequent request.
