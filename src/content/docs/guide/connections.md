---
title: "Connections"
description: "Manage connection reuse, open responses, request errors, timeouts, and buffered writes."
route: "/en/guide/connections.html"
section: "guide"
---

# Connections

`app.process(&client)` handles one request. It parses the request, dispatches
handlers, runs the final callback, and finalizes the response before returning
an `App::ProcessResult`. aWOT borrows the client during processing; neither
result flag extends the client's lifetime. After a successful response,
connection cleanup depends on the client implementation and whether your
application keeps a client object alive.

<h2 id="close">Close after a response</h2>


A connection can close explicitly through `client.stop()` or automatically
when its owning client is destroyed. Scope-exit behavior depends on the
networking library used with the board:

| Client implementation | When a local client goes out of scope |
| --- | --- |
| ESP32 `WiFiClient` / `NetworkClient` (Arduino-ESP32 3.3.10) | The socket closes when the last shared owner is destroyed. If no other copy is retained, a local client needs no explicit `stop()` before leaving scope. |
| Arduino Ethernet 2.0.2 `EthernetClient`, for example on an Uno with an Ethernet shield | The destructor does not close the socket. Call `stop()` when finished; leaving scope alone can leave the connection open. |

See the [ESP32 socket ownership implementation](https://github.com/espressif/arduino-esp32/blob/3.3.10/libraries/Network/src/NetworkClient.cpp#L163-L203),
the [Ethernet client destructor](https://github.com/arduino-libraries/Ethernet/blob/2.0.2/src/Ethernet.h#L192-L208),
and [Ethernet `stop()`](https://github.com/arduino-libraries/Ethernet/blob/2.0.2/src/EthernetClient.cpp#L124-L143).
Other client libraries can make different choices, including on the same board.

The [hello world sketches](/en/starter/hello-world.html) use explicit closure
for a close-after-response policy that works with either implementation. This
partial loop fragment belongs after acquiring a connected client:

```cpp
app.process(&client);
client.stop();
```

This is appropriate for an ordinary response whose body ends when the
connection closes. On ESP32, letting the last local client leave scope after
processing can provide the same closure. HTTP/1.1 requests alone do not enable
reuse. aWOT defaults to `Connection: close` unless the response explicitly
requests otherwise; this header does not itself call `client.stop()`.

<h2 id="keep-alive">Keep-Alive</h2>


Keep-alive lets a browser send another request on the same TCP connection
instead of opening a new one for every request. A browser can also open several
connections at once, so the server needs to manage each connection separately.

Set `Connection: keep-alive` before the headers are sent, consume the complete
request body, and check `result.keepAlive` after processing. It is true only
after successful parsing and writes, with no unread body and no open response.
If the response has no explicit Content-Length, aWOT supplies chunk framing.
The caller still controls closure and the idle timeout.

On ESP32, keep at least one `WiFiClient` owner alive between calls to `loop()`.
Store the client at file scope, in a `static` variable, or in a persistent
client pool. Skipping `client.stop()` when `keepAlive` is true does not keep a
local client alive.
ESP32 client copies share the socket; calling `stop()` on any copy closes it
for all owners.

This complete ESP32 sketch keeps up to six connections open. Each has its own
client handle and idle timer. Every pass through `loop()` checks all slots,
accepts new connections into free slots, and processes requests that have data
ready. An idle connection can stay open while another client sends a request.
Connections close after five seconds without activity.

Set the Wi-Fi credentials, upload, and visit `/`. A request body on this GET
route is deliberately left unread, which causes `keepAlive` to be false and
the caller to close that client.

```cpp
#include <WiFi.h>
#include <aWOT.h>

WiFiServer server(80);
App app;

struct ClientState {
  WiFiClient client;
  unsigned long lastActivity = 0;
};

const unsigned int maxClients = 6;
ClientState clients[maxClients]; // Survive returning from loop().
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
  for (ClientState& slot : clients) {
    if (!slot.client.connected()) {
      slot.client.stop();
      slot.client = server.accept();
      slot.lastActivity = millis();
    }
    if (!slot.client.connected()) continue;

    if (slot.client.available()) {
      App::ProcessResult result = app.process(&slot.client);
      slot.lastActivity = millis();
      if (!result.keepAlive) slot.client.stop();
    } else if (millis() - slot.lastActivity >= maxIdle) {
      slot.client.stop();
    }
  }
}
```

To request the page twice on one connection, replace the address below with
your board's address and run both URLs in the same curl invocation:

```bash
curl --http1.1 --verbose http://192.168.1.140/ http://192.168.1.140/
```

Check that curl reports reusing the connection for its second request. Two
separate curl processes establish separate connections.

The connections stay open together, but this loop processes requests one at a
time. `app.process()` is synchronous: an incomplete request or a slow handler
can delay the other clients. The request timeout limits waits for missing
input; handlers should return promptly.

Choose `maxClients` for your board's available memory and network limits. When
all slots are occupied, the sketch waits for a slot to become free before
accepting another connection.

This example serves ordinary responses and closes clients when `keepAlive` is
false. If you add streaming or protocol upgrades, handle `responseOpen`
separately as shown below; that connection belongs to the ongoing response or
protocol and must not be passed back to `process()` for another HTTP request.

<h2 id="open-responses">Continue a response after process()</h2>


Call `res.keepOpen()` **before sending ordinary HTTP headers**, then emit your
initial response. `result.responseOpen` is true when the request parsed and the
response writes succeeded. Retain the concrete client and use its write
methods afterward; `Request` and `Response` expire when `process()` returns.
The sketch below copies the accepted handle into the global `streamClient`
before `incoming` leaves scope. That retained copy keeps the ESP32 socket alive.

For HTTP streaming, write directly to the client and close the connection when
you are done. This tells the browser the response is complete. For a protocol
upgrade such as WebSockets, keep the connection open and use it for the new
protocol.

Here is a complete ESP32 sketch that streams a few lines over five seconds:

```cpp
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
[Server Sent Events](/en/guide/server-sent-events.html); their guides show
upgrades, polling, and cleanup.

<h2 id="errors">Errors, missing routes, and finally</h2>


`app.onError(handler)` receives parser errors and the built-in missing-route
error. A custom response **must call `res.end()`**; otherwise aWOT supplies its
built-in error response after the callback. This partial example goes at file
scope, with the registration inside `setup()`:

```cpp
void httpError(Request& req, Response& res, int statusCode) {
  res.status(statusCode);
  res.set("Content-Type", "application/json");
  res.print("{\"error\":");
  res.print(statusCode);
  res.print("}");
  res.end();
}
```

```cpp
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
