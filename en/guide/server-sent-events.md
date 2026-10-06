---
layout: page
title: Server-sent events with aWOT 4
menu: guide
lang: en
---

# Server-sent events

Server-sent events send a stream of text updates from the Arduino to a browser.
Use a normal HTTP endpoint for commands sent in the other direction, or use
[WebSockets](/en/guide/websockets.html) for bidirectional messages. Install
aWOT **4.0.0** through Arduino Library Manager. This complete ESP32 sketch uses
the explicit `<SseChannel.h>` header and serves a page showing uptime and a
named, multiline event. Set your WiFi credentials and open the address printed
at 115200 baud.

```arduino
#include <WiFi.h>
#include <aWOT.h>
#include <SseChannel.h>

WiFiServer server(80);
App app;
SseChannel<WiFiClient, 4> events;
unsigned long lastPush = 0;

P(page) =
    "<!doctype html><meta charset='utf-8'><title>aWOT events</title>"
    "<p id='status'>Connecting...</p><p>Uptime: <span id='uptime'></span></p>"
    "<pre id='details'></pre><script>"
    "const source=new EventSource('/events');"
    "source.onopen=()=>document.getElementById('status').textContent='Connected';"
    "source.onerror=()=>document.getElementById('status').textContent="
    "'Connection interrupted; browser will retry';"
    "source.onmessage=e=>document.getElementById('uptime').textContent="
    "e.data+' seconds';"
    "source.addEventListener('sample',e=>"
    "document.getElementById('details').textContent=e.data);"
    "window.addEventListener('pagehide',()=>source.close());"
    "</script>";

void indexHandler(Request& req, Response& res) {
  res.set("Content-Type", "text/html; charset=utf-8");
  res.printP(page);
}

void eventsHandler(Request& req, Response& res) {
  events.open(res);
}

void onConnect(Print& client) {
  Serial.println("Event stream added");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());

  events.onConnect(onConnect);
  app.get("/", indexHandler);
  app.get("/events", eventsHandler);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client.connected()) {
    App::ProcessResult result = app.process(&client);
    if (result.responseOpen) {
      events.add(client);
    } else {
      client.stop();
    }
  }

  unsigned long now = millis();
  if (now - lastPush >= 1000) {
    lastPush = now;
    char uptime[16];
    snprintf(uptime, sizeof(uptime), "%lu", now / 1000);
    events.send(uptime);
    events.send("sample", "Arduino is online\nUpdated once per second");
  }
}
```

## Opening and retaining the connection

Call `events.open(res)` before sending headers. It selects status 200,
`text/event-stream`, and `Cache-Control: no-cache`, then calls `keepOpen()`.
The HTTP body is close-delimited, without Content-Length or chunk framing.
`App::process()` sends the headers and returns `responseOpen == true` after
successful processing. Only then does the sketch call `events.add(client)`.

`add()` copies the concrete client handle into one of the four slots and calls
`onConnect(Print&)`. The copy shares the original connection, so leave the
original handle unstopped after handing it to the channel. If every slot is
connected, `add()` stops the new client. Disconnected slots are reusable.
Use a channel capacity that fits your transport's socket count and board RAM.
Keep the channel alive while its connections are active.

Match the channel's client type to the accepted transport: this sketch uses
`WiFiClient` throughout. Do not add the temporary adapter from
`process(Stream*)`; it and the HTTP Request/Response objects are temporary.
For applications with several open-response routes, track which route opened
the response, for example through request context, before assigning it to an
SSE channel. This sketch has one such route.

## Events and browser handling

`send(data)` broadcasts an unnamed event handled by `EventSource.onmessage`.
`send(event, data)` sends a named event handled with `addEventListener(event,
...)`. Both accept non-null NUL-terminated strings; event names must contain
no CR or LF. They send immediately and do not retain the strings. Send UTF-8
text; binary bytes or embedded NUL need an application encoding.

The multiline sample is sent as two `data:` lines followed by a blank line.
The channel normalizes CR, LF, and CRLF, preserving empty and trailing lines.
The browser joins the lines into the event's `data` string. Use `textContent`
when displaying received text, as the example does. These event types and text
encoding follow the [EventSource standard](https://html.spec.whatwg.org/multipage/server-sent-events.html#server-sent-events).

An equivalent browser client for a page already served by the device is:

```javascript
const events = new EventSource('/events');
events.onmessage = (event) => console.log('Uptime:', event.data);
events.addEventListener('sample', (event) => console.log(event.data));
// Call events.close() when the page no longer needs updates.
```

## Disconnects, heartbeat, and replay

There is no SSE `poll()` method. `send()` detects disconnected clients and
write failures, stops the affected handles, and frees their slots. It retries
positive short writes but returns no delivery result. `forEach(callback)`
also removes disconnected clients and exposes each remaining client's Print
interface, which you can use for custom SSE comments or fields. Direct writes
through that interface need their own return-count/error handling. The channel
has an `onConnect` callback and no disconnect callback or public connection
count.

The once-per-second uptime is application traffic. The channel has no automatic
heartbeat and does not send periodic comments by itself. If an application is
otherwise silent, schedule your own comment or event writes and cleanup.

[`EventSource` reconnects after an interrupted stream](https://html.spec.whatwg.org/multipage/server-sent-events.html#processing-model), but this sketch resumes
with the current uptime. The channel does not generate event IDs, retain an
event history, or replay missed updates. Reliable replay requires application
logic: capture `Last-Event-ID` in a `Request::Header` array, retain a bounded
history, and emit appropriate `id:` fields. See the
[SseChannel reference](/en/4x/api.html#sse) for all public methods.
