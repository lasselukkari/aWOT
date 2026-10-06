---
layout: page
title: aWOT connections and response lifecycle
menu: guide
lang: en
---

# Connections and responses

`app.process(&client)` handles one request. It parses the request, dispatches
handlers, runs the final callback, and finalizes the response before returning
an `App::ProcessResult`. The application owns the client and decides whether
to retain it or call `client.stop()`.

<h2 id="close">Close after a response</h2>

The [hello world sketches](/en/starter/hello-world.html) close every connection.
This partial loop fragment belongs after acquiring a connected client:

```arduino
app.process(&client);
client.stop();
```

This is appropriate for an ordinary response whose body ends when the
connection closes. HTTP/1.1 requests alone do not enable reuse. aWOT defaults
to `Connection: close` unless the response explicitly requests otherwise.

<h2 id="keep-alive">Reuse a connection</h2>

Set `Connection: keep-alive` before the headers are sent, consume the complete
request body, and check `result.keepAlive` after processing. It is true only
after successful parsing and writes, with no unread body and no open response.
If the response has no explicit Content-Length, aWOT supplies chunk framing.
The caller still controls closure and the idle timeout.

This complete ESP32 sketch retains one connection between requests and closes
it after five seconds without activity. Set the Wi-Fi credentials, upload, and
visit `/`. A request body on this GET route is deliberately left unread, which
causes `keepAlive` to be false and the caller to close the client.

```arduino
#include <WiFi.h>
#include <aWOT.h>

WiFiServer server(80);
WiFiClient client;
App app;
unsigned long lastActivity = 0;
const unsigned long maxIdle = 5000;

void allowReuse(Request& req, Response& res) {
  res.set("Connection", "keep-alive");
  req.next();
}

void hello(Request& req, Response& res) {
  res.print("Hello World!");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());
  app.setTimeout(1000);
  app.use(allowReuse);
  app.get("/", hello);
  server.begin();
}

void loop() {
  if (!client.connected()) {
    client.stop();
    client = server.available();
    lastActivity = millis();
  }
  if (!client.connected()) return;

  if (client.available()) {
    App::ProcessResult result = app.process(&client);
    lastActivity = millis();
    if (!result.keepAlive) client.stop();
  } else if (millis() - lastActivity >= maxIdle) {
    client.stop();
  }
}
```

For a larger client pool, retain a separate concrete client handle and activity
timestamp in each slot. Check available input before calling `process()` again
on a retained idle client. Never begin a new HTTP request on a client whose
`result.responseOpen` was true; it belongs to the open response or protocol.

<h2 id="open-responses">Continue a response after process()</h2>

Call `res.keepOpen()` **before sending ordinary HTTP headers**, then emit your
initial response. `result.responseOpen` is true when the request parsed and the
response writes succeeded. Retain the concrete client and use its write
methods afterward; `Request` and `Response` expire when `process()` returns.

Ordinary open HTTP responses are close-delimited. aWOT forces `Connection:
close`, removes Content-Length and Transfer-Encoding when emitting headers,
and leaves the transport available for your streaming code. Finish by stopping
the client. A `101 Switching Protocols` response instead retains its upgrade
headers and never emits a terminal HTTP chunk.

Here is a complete ESP32 sketch that streams a few lines over five seconds:

```arduino
#include <WiFi.h>
#include <aWOT.h>

WiFiServer server(80);
WiFiClient streamClient;
App app;
unsigned long started = 0;
unsigned long lastLine = 0;

void openStream(Request& req, Response& res) {
  res.keepOpen();
  res.set("Content-Type", "text/plain");
  res.print("Stream opened\n");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());
  app.get("/stream", openStream);
  server.begin();
}

void loop() {
  if (!streamClient.connected()) {
    streamClient.stop();
    WiFiClient incoming = server.available();
    if (incoming.connected()) {
      App::ProcessResult result = app.process(&incoming);
      if (result.responseOpen) {
        streamClient = incoming;
        started = lastLine = millis();
      } else {
        incoming.stop();
      }
    }
  }

  if (!streamClient.connected()) return;
  unsigned long now = millis();
  if (now - started >= 5000) {
    streamClient.stop();
  } else if (now - lastLine >= 1000) {
    const char line[] = "Still streaming\n";
    size_t offset = 0;
    while (offset < sizeof(line) - 1) {
      size_t written = streamClient.write(
          (const uint8_t*)line + offset, sizeof(line) - 1 - offset);
      if (written == 0) {
        streamClient.stop();
        break;
      }
      offset += written;
    }
    lastLine = now;
  }
}
```

```bash
curl --no-buffer http://192.168.1.140/stream
```

The write loop handles short writes on the retained client. Once outside
`process()`, the caller handles transport failures directly. Use the channel
helpers for [WebSockets](/en/guide/websockets.html) and
[server-sent events](/en/guide/server-sent-events.html); their guides show
upgrades, polling, and cleanup.

<h2 id="errors">Errors, missing routes, and finally</h2>

`app.onError(handler)` receives parser errors and the built-in missing-route
error. A custom response **must call `res.end()`**; otherwise aWOT supplies its
built-in error response after the callback. This partial example goes at file
scope, with the registration inside `setup()`:

```arduino
void httpError(Request& req, Response& res, int statusCode) {
  res.status(statusCode);
  res.set("Content-Type", "application/json");
  res.print("{\"error\":");
  res.print(statusCode);
  res.print("}");
  res.end();
}
```

```arduino
// Add inside setup().
app.onError(httpError);
```

`app.notFound(handler)` handles missing routes separately and takes precedence
over `onError` for those requests. aWOT sets status 404 before calling it.
The built-in missing-route decision occurs when no response status was sent
and the response was not ended, so a handler that returns without output can
still produce a 404.

`app.finally(handler)` runs after routing or a parser error, before response
finalization, even when a handler ended the response. It may read context and
record the status, but write errors can still occur during finalization after
it returns. Invalid processing options bypass all callbacks and stop the client.
Parser-error callbacks may see incomplete request metadata; do not assume that
path, version, headers, or body have been parsed successfully.

<h2 id="timeouts">Request and idle timeouts</h2>

`app.setTimeout(milliseconds)` sets the timeout used by the parser and the
request's inherited Stream reads; the default is 1000 milliseconds. Parser
timeouts produce 408 and close the client. Handlers must detect incomplete
body reads themselves. This timeout applies to waiting for input during a
request, rather than an entire connection lifetime. The sketch owns idle
timeouts, open-stream deadlines, and eventual client closure.

<h2 id="writes">Buffer acceptance and delivery</h2>

`res.write()` and Arduino `Print` methods can report bytes accepted into aWOT's
output buffer before they reach the transport. Positive short writes are
retried from the accepted offset. A zero transport write sets inherited
`res.getWriteError()` and closes the client. A later `flush()` or finalization
can also fail; failed responses return neither keepAlive nor responseOpen.

`res.bytesSent()` counts accepted status, header, and payload bytes and
saturates at `INT_MAX`; it does not confirm receipt by the peer. Use write
results and `getWriteError()` for application diagnostics, and the result from
`process()` for the final connection decision.

HEAD and statuses 204, 205, and 304 suppress payload output and chunk
terminators. Interim statuses 100, 102, and 103 flush an interim response and
reset response state for a final response; headers queued before the interim
status belong to that interim response. `res.setDefaults()` is a compatibility
no-op; current defaults are selected when headers are emitted.
