---
layout: page
title: Configuring aWOT 4 memory and callbacks
menu: guide
lang: en
---

# Configuration

aWOT 4 uses fixed HTTP buffers and caller-owned views. Install aWOT **4.0.0**
through Arduino Library Manager. Choose capacities for your board and longest
request, and keep borrowed storage alive until `process()` returns.

## Per-request storage on Uno

This complete Uno sketch uses the Ethernet 2.x library and a static IP address.
Change the address and MAC for your network. Its 64-byte URL buffer and
32-byte output buffer match aWOT's Uno defaults. Supplying them explicitly
replaces the default arrays for this request; it does not add another default
copy. The response writes a string stored in program memory.

```arduino
#include <Ethernet.h>
#include <aWOT.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);
App app;
P(message) = "Hello from flash!";

void hello(Request& req, Response& res) {
  res.printP(message);
}

void setup() {
  Ethernet.begin(mac, ip);
  app.get("/", hello);
  server.begin();
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    char url[64];
    uint8_t output[32];
    App::ProcessOptions options;
    options.urlBuffer = url;
    options.writeBuffer = output;
    App::ProcessResult result = app.process(&client, options);
    if (!result.responseOpen && !result.keepAlive) client.stop();
  }
}
```

These assignments infer array capacities through `ArrayView`. If you have only
a pointer, assign `{pointer, capacity}` instead. Null URL or output pointers
select defaults. A custom URL capacity must be at least two bytes, and custom
output capacity must be greater than `CHUNK_OVERHEAD`; invalid options close
the client without running handlers or `finally`. Header capture entries borrow
their names and buffers too. See the [request-data guide](/en/guide/request-data.html)
for headers and context.

Output capacity includes reserved chunk framing. At Uno's 32-byte default,
`CHUNK_HEADER_SIZE` is four, the trailer is two, and at most 26 buffered payload
bytes fit. Larger writes are split and flushed. A buffered write's returned
count means bytes accepted into the buffer; later transport failure can still
occur during flush/finalization. Inspect `getWriteError()` inside handlers when
appropriate, and honor the returned processing result in the connection owner.

`P(name)` declares a static unsigned-character array in program memory.
`res.printP(name)` prints a NUL-terminated flash string;
`res.writeP(name, length)` writes an explicit number of bytes. This saves AVR
RAM for suitable constant data. It does not make ordinary request/response
header pointers into flash-aware strings.

## HTTP defaults and build-wide overrides

| Setting | Uno/Leonardo default | Other default |
| --- | --- | --- |
| `SERVER_URL_BUFFER_SIZE` | 64 | 256 |
| `SERVER_PUSHBACK_BUFFER_SIZE` | 32 | 128 |
| `SERVER_OUTPUT_BUFFER_SIZE` | 32 | 1024 |
| `SERVER_MAX_HEADERS` | 10 | 10 |

`SERVER_MAX_HEADERS` limits response headers supplied with `res.set()`, not
captured request headers. Built-in default headers do not use these slots.
Requests reserve their own pushback array. Total memory also includes the
transport library, route registrations, application state, and stack.

Prefer `ProcessOptions` for URL/output storage. If you need to change the
compiled defaults, pushback size, or response header capacity, supply compiler
defines to **every library and sketch translation unit**. These settings affect
object layout or library behavior. A define at the beginning of an `.ino`
does not reconfigure separately compiled `App.cpp`, `Request.cpp`, and
`Response.cpp`. Rebuild all affected files with consistent flags.

For example, the following command assumes the board core is installed and
uses the installed aWOT library. It selects function-pointer callbacks and
smaller HTTP defaults throughout the C++ build:

```bash
arduino-cli compile --fqbn esp32:esp32:nodemcu-32s \
  --build-property 'compiler.cpp.extra_flags=-DAWOT_NO_STD_FUNCTION_MIDDLEWARE -DSERVER_URL_BUFFER_SIZE=64 -DSERVER_PUSHBACK_BUFFER_SIZE=32 -DSERVER_OUTPUT_BUFFER_SIZE=64 -DSERVER_MAX_HEADERS=8' \
  /path/to/Sketch
```

Preserve any existing project compiler flags when adding these. Inspect the
verbose compiler commands for both the sketch and library to confirm the
flags are applied; board cores can have different build properties. Use valid
positive capacities, at least two URL bytes, and output capacity greater than
its derived `CHUNK_OVERHEAD`. See the [configuration reference](/en/4x/api.html#configuration-http)
for all derived chunk macros.

## Functions, lambdas, and member callbacks

aWOT automatically uses `std::function` when supported by the platform, including
ESP32 and ESP8266 or toolchains that can include `<functional>`. It accepts
functions, capturing lambdas, and the App/Router member-binding overloads.
Keep referenced objects and bound instances alive for the lifetime of their
registrations. Lambda captures can allocate additional memory through
`std::function`, depending on the toolchain and capture size.

For a function-pointer build, use `-DAWOT_NO_STD_FUNCTION_MIDDLEWARE` consistently
as above. Free/static functions and lambdas without captures work; capturing
lambdas and member-binding overloads are unavailable. For example, the `hello`
function in the complete sketch works in either configuration. Defining both
`AWOT_NO_STD_FUNCTION_MIDDLEWARE` and `STD_FUNCTION_MIDDLEWARE` is an error.
The choice also affects WebSocket and SSE callback types. `App::onError`
always accepts a function pointer.

All types live in `namespace awot`. `<aWOT.h>` adds a convenience using-directive
unless you define `ENABLE_AWOT_NAMESPACE` before including it. With that macro,
write qualified names such as `awot::App` and `awot::Request`. This changes
name lookup in the including source, not the library's namespace or ABI.

## Channel capacity and heartbeat

Include channels explicitly. For example, the Uno
[SerialBridge example](https://github.com/lasselukkari/aWOT/blob/4/examples/SerialBridge/SerialBridge.ino)
sets `WS_RX_BUFFER_SIZE` to 64 before `<WsChannel.h>` and uses one Ethernet
client slot and one room. Each WebSocket slot reserves the entire receive
buffer, whether or not it is occupied. Four 256-byte slots alone reserve 1024
payload bytes, before slot metadata and HTTP/transport storage; that choice
is unsuitable for many small-board applications.

`WsChannel<TClient, N, MAX_ROOMS>` requires `N > 0` and `1 <= MAX_ROOMS <= 32`.
Its defaults are a 256-byte receive buffer, a 5000 ms ping interval, and a
10000 ms pong timeout. `WS_PING_INTERVAL_MS` and `WS_PING_TIMEOUT_MS` configure
heartbeat; zero does not turn it off. Define channel macros before the channel
header and use the same values across translation units instantiating the
same channel type.

`SseChannel<TClient, N>` stores `N` client handles and has no automatic
heartbeat or replay buffer. Both channel types copy concrete client handles;
the template type must match the accepted transport, and your connection owner
must hand open responses to the appropriate channel. See the
[WebSocket guide](/en/guide/websockets.html) and
[SSE guide](/en/guide/server-sent-events.html) for complete lifecycles.
