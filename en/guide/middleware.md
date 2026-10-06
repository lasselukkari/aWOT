---
layout: page
title: Writing middleware for use in aWOT apps
menu: guide
lang: en
---

# Middleware

Middleware receives the current `Request` and `Response`, just like a route
handler. Registrations execute in order. In aWOT 4, call `req.next()` to pass
control to the next matching registration. Returning without it stops dispatch.
`res.end()` marks a response as ended and stops dispatch even when continuation
was requested. The final callback still runs before aWOT finalizes the output.

<h2 id="example">Logging and authentication</h2>

This complete ESP32 sketch logs each request and protects `/secret` with HTTP
Basic authentication. Install [aWOT 4.0.0](/en/starter/hello-world.html) and set
the Wi-Fi credentials. The demonstration credentials are `super` / `admin`.
Basic authentication sends reusable credentials; use a protected transport or
network for an application that relies on it.

```arduino
#include <WiFi.h>
#include <aWOT.h>
#include <string.h>

WiFiServer server(80);
App app;

void logRequest(Request& req, Response& res) {
  Serial.println(req.path());
  req.next();
}

void authenticate(Request& req, Response& res) {
  const char* authorization = req.get("Authorization");
  if (!authorization ||
      strcmp(authorization, "Basic c3VwZXI6YWRtaW4=") != 0) {
    res.set("WWW-Authenticate", "Basic realm=\"Secret Area\"");
    res.sendStatus(401);
    res.end();
    return;
  }
  req.next();
}

void secret(Request& req, Response& res) {
  res.print("Welcome to the secret area.");
}

void home(Request& req, Response& res) {
  res.print("Visit /secret with super / admin.");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());
  app.use(logRequest);
  app.use("/secret", authenticate);
  app.get("/secret", secret);
  app.get("/", home);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client.connected()) {
    char authorization[64];
    Request::Header headers[] = {
      {"Authorization", authorization}
    };
    app.process(&client, headers);
    client.stop();
  }
}
```

The logger calls `req.next()` so authentication and routes can run. Authentication
calls it only after a successful check. Its `401` branch calls `res.end()` and
returns, so `secret` cannot execute. Put middleware before the handlers that
depend on it. The `/secret` callback pattern also accepts `/secret/`; protecting
a group of paths is best done with middleware inside a
[mounted router](/en/guide/routing.html#aWOT-router).

Header capture belongs to each `process()` call. The `Request::Header` array
borrows its names and buffers, infers the array sizes, and clears the captured
values before and after processing. Keep them alive throughout the call and
use separate storage for concurrent requests. `req.get()` returns null for an
absent or uncaptured header, and a non-null empty string for a captured header
with an empty value. See [request data](/en/guide/request-data.html).

Replace the IP address below with the address printed by your board:

```bash
curl -i http://192.168.1.140/secret
curl -i --user super:admin http://192.168.1.140/secret
```

The first command receives `401`; the second receives the welcome message.
