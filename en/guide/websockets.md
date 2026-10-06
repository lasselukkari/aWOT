---
layout: page
title: WebSockets with aWOT 4
menu: guide
lang: en
---

# WebSockets

WebSockets let the browser and Arduino send messages over the same connection.
Install aWOT **4.0.0** through Arduino Library Manager and use the explicit
`<WsChannel.h>` header. This complete ESP32 sketch serves a browser page and
broadcasts each message to a room of up to four clients. Set your WiFi
credentials, upload it, and open the IP address printed at 115200 baud.

```arduino
#include <WiFi.h>
#include <aWOT.h>
#include <WsChannel.h>

WiFiServer server(80);
App app;
WsChannel<WiFiClient, 4, 1> sockets;
int chatRoom = -1;

P(page) =
    "<!doctype html><meta charset='utf-8'><title>aWOT WebSockets</title>"
    "<p id='status'>Connecting...</p><input id='message'>"
    "<button id='send' disabled>Send</button><pre id='log'></pre>"
    "<script>"
    "const status=document.getElementById('status');"
    "const input=document.getElementById('message');"
    "const button=document.getElementById('send');"
    "const log=document.getElementById('log');"
    "const ws=new WebSocket('ws://'+location.host+'/ws');"
    "ws.binaryType='arraybuffer';"
    "ws.onopen=()=>{status.textContent='Connected';button.disabled=false;};"
    "ws.onclose=()=>{status.textContent='Disconnected';button.disabled=true;};"
    "ws.onerror=()=>{status.textContent='Connection error';};"
    "ws.onmessage=e=>{log.textContent+=(typeof e.data==='string'?e.data:"
    "'Binary: '+Array.from(new Uint8Array(e.data)).join(','))+'\\n';};"
    "button.onclick=()=>{"
    "if(ws.readyState!==WebSocket.OPEN)return;"
    "if(new TextEncoder().encode(input.value).length>256){"
    "status.textContent='Use at most 256 UTF-8 bytes';return;}"
    "ws.send(input.value);input.value='';};"
    "</script>";

void indexHandler(Request& req, Response& res) {
  res.set("Content-Type", "text/html; charset=utf-8");
  res.printP(page);
}

void websocketHandler(Request& req, Response& res) {
  if (!sockets.upgrade(req, res)) res.sendStatus(400);
}

void onConnect(int socketId) {
  sockets.join(socketId, chatRoom);
  sockets.send(socketId, "Welcome!");
}

void onDisconnect(int socketId) {
  Serial.print("Disconnected slot: ");
  Serial.println(socketId);
}

void onMessage(WsMessage& msg) {
  sockets.to(chatRoom, msg.data(), msg.length(), msg.isBinary());
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());

  chatRoom = sockets.defineRoom("chat");
  if (chatRoom < 0) {
    Serial.println("No room capacity");
    while (true) delay(1000);
  }
  sockets.onConnect(onConnect);
  sockets.onDisconnect(onDisconnect);
  sockets.onMessage(onMessage);
  app.get("/", indexHandler);
  app.get("/ws", websocketHandler);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client.connected()) {
    char upgrade[16], connection[64], key[25], version[4];
    Request::Header headers[] = {
        {"Upgrade", upgrade},
        {"Connection", connection},
        {"Sec-WebSocket-Key", key},
        {"Sec-WebSocket-Version", version}};
    App::ProcessResult result = app.process(&client, headers);
    if (!result.responseOpen) client.stop();
  }
  sockets.poll();
}
```

## Upgrade and connection ownership

The four `Request::Header` entries capture the handshake fields for this
request. Array capacities include the terminating NUL. Their names and buffers
remain alive throughout `process()`, and the library clears values before and
after processing. Use separate capture storage for simultaneous requests.
`upgrade()` requires GET, HTTP/1.1, an empty/consumed request body, the WebSocket
13 handshake, and a free channel slot. It returns false for an invalid
handshake, insufficient capacity, or a write failure.

On success, the channel writes the 101 response and copies the concrete
`WiFiClient` handle. The copy refers to the same underlying connection. The
`ProcessResult.responseOpen` check leaves that connection under channel
management; calling `client.stop()` on the original handle would also close
it. Other requests use ordinary responses and are closed explicitly by this
sketch. Use the [connection guide](/en/guide/connections.html) if you also
want HTTP keep-alive routes.

The channel's client template argument must match the exact concrete type
passed to `process()`. `upgrade()` casts `req.client()` to that type. Do not
use `process(Stream*)` for an upgrade: its adapter exists only until processing
returns. Keep the channel and its callbacks alive while connections are active.
Avoid handing a channel-managed connection back to the HTTP parser.

## Polling, messages, and rooms

Call `poll()` frequently, including when no new HTTP client is available. It
collects partial frame bytes, calls message callbacks, replies to control
frames, detects disconnects, and handles heartbeat pings. Blocking handlers or
long delays postpone all of those tasks.

`WsMessage` is a Stream, so you can read it byte by byte or pass it to a parser.
Its `data()` pointer and `length()` describe the complete received payload,
even after reads. They are valid only during the callback; copy the bytes if
you need them later. Payloads are not NUL-terminated and binary messages can
contain zero bytes. The room broadcast therefore uses the explicit length and
preserves the binary/text flag.

`send(socketId, ...)` targets one active client, `broadcast(...)` targets all
active clients, and `to(room, ...)` targets room members. `defineRoom()` returns
an index or `-1` when definitions are exhausted. Room names are borrowed; this
sketch uses a permanent string literal. `join()` and `leave()` update membership;
one client can belong to several rooms. A new connection starts with no rooms.
There can be at most 32 rooms, and the chosen `MAX_ROOMS` template parameter
sets the actual room capacity.

Socket IDs are reusable slot indices. `connected(id)` and `count()` report
active slots; `poll()` updates them after transport disconnects. Call
`close(id)` to send code 1000 and stop a connection, or supply another supported
close code. `onDisconnect` runs when a slot is deactivated, including write
failures. The sending methods return void, retry positive short writes, and
stop a client if a write fails.

## Capacity, masking, and heartbeat

Each slot reserves 256 receive bytes by default. Incoming payloads larger than
that close with code 1009. The browser page limits UTF-8 byte length, which can
be greater than the number of displayed characters. Incoming data frames must
be unfragmented and masked. Browser WebSocket clients handle masking; custom
clients must supply it. Fragmented data and continuation frames close with
1003, protocol errors with 1002, and invalid UTF-8 with 1007. Compression
extensions are unsupported. This sketch serves plain `ws://`; it does not add
TLS for `wss://`.

The channel sends a ping every 5000 ms when none is outstanding, and stops a
client if its pong has not arrived within 10000 ms. Browsers answer ping frames
without JavaScript involvement; these control frames are outside the
[browser WebSocket API](https://websockets.spec.whatwg.org/#ping-and-pong-frames).
Change `WS_RX_BUFFER_SIZE`,
`WS_PING_INTERVAL_MS`, and `WS_PING_TIMEOUT_MS` before including the channel
header, consistently across translation units that instantiate the same
channel. Zero does not disable heartbeat. Outgoing frames use explicit lengths
and are not constrained by the receive capacity.

An Uno has much less RAM than an ESP32. For Uno with Ethernet, start with the
library's [SerialBridge example](https://github.com/lasselukkari/aWOT/blob/4/examples/SerialBridge/SerialBridge.ino):
it uses `WsChannel<EthernetClient, 1, 1>` and a 64-byte receive limit. With the
Ethernet 2.x library, `server.accept()` transfers accepted clients out of the
server's pending list, so later HTTP acceptance does not reuse an upgraded
socket. See the [Ethernet implementation](https://github.com/arduino-libraries/Ethernet/blob/master/src/EthernetServer.cpp)
for the difference between `accept()` and `available()`.

See the complete [WsChannel reference](/en/4x/api.html#ws) and
[WsMessage reference](/en/4x/api.html#wsmessage) for signatures and callbacks.
