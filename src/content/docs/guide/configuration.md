---
title: "Configuring aWOT 4 memory and callbacks"
description: "Configure aWOT memory buffers, HTTP defaults, callbacks, and channel capacities."
route: "/en/guide/configuration.html"
section: "guide"
---

# Configuration

aWOT provides URL and output buffers automatically. Use `App::ProcessOptions`
to choose their sizes and where they are stored.

<span id="per-request-storage-on-uno"></span>

## Use your own buffers

Supply a larger URL buffer when requests include long paths or query strings.
For output, a smaller buffer saves RAM, while a larger one can reduce the
number of writes to the client. The output buffer does not limit the size of
the response: aWOT sends the data in parts as the buffer fills.

Static or global arrays also let you keep these buffers off the stack. They
reserve RAM for the lifetime of the program, so this changes where memory is
used rather than saving RAM by itself.

This complete Uno sketch uses a 128-byte URL buffer and a 64-byte output
buffer, twice the Uno defaults. The `static` arrays are reused for each request.
It uses the Ethernet 2.x library and a static IP address; change the address
and MAC for your network.

```cpp
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
    static char url[128];
    static uint8_t output[64];
    App::ProcessOptions options;
    options.urlBuffer = url;
    options.writeBuffer = output;
    App::ProcessResult result = app.process(&client, options);
    if (!result.responseOpen && !result.keepAlive) client.stop();
  }
}
```

Assigning an array sets its capacity automatically. If you only have a pointer,
use `{pointer, capacity}`. You can supply either buffer independently; aWOT uses
its default for any buffer you leave unset. Each supplied buffer replaces the
corresponding default array.

Keep the buffers alive until `process()` returns. Reuse them for requests
processed one after another, as above; calls running at the same time need
separate buffers.

The URL buffer must fit the encoded path and query string plus a terminating
zero byte. Its capacity must be at least two bytes. The output buffer reserves
some space for HTTP chunk framing and must be larger than `CHUNK_OVERHEAD`.
See the [buffer reference](/en/4x/api.html#configuration-http) for the limits.

### Store constant text in flash

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
[SerialBridge example](https://github.com/lasselukkari/aWOT/blob/master/examples/SerialBridge/SerialBridge.ino)
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
[Server Sent Events guide](/en/guide/server-sent-events.html) for complete lifecycles.
