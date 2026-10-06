# aWOT 4.0

An Arduino HTTP server library that works with Arduino `Client` and `Stream`
transports. Routes and middleware receive a `Request` and a `Response`; both
support the Arduino stream/print interfaces.

## Arduino Uno and Ethernet

Install aWOT and the Arduino Ethernet library. This example uses DHCP with a
W5100 or W5500 shield:

```cpp
#include <Ethernet.h>
#include <aWOT.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetServer server(80);
App app;

void indexHandler(Request& req, Response& res) {
  res.print(F("Hello World!"));
}

void setup() {
  Ethernet.begin(mac);
  app.get("/", indexHandler);
  server.begin();
}

void loop() {
  EthernetClient client = server.available();
  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
```

The [Ethernet example](examples/Ethernet/Ethernet.ino) serves a page stored in
program memory. The [SerialBridge example](examples/SerialBridge/SerialBridge.ino)
uses Ethernet 2.x, a static IP, one WebSocket client, and a 64-byte receive buffer
for Uno. Change its IP address to suit your network.

[WiFi](examples/WiFi/WiFi.ino), [member callbacks](examples/MemberFunction/MemberFunction.ino),
[request context](examples/RequestContext/RequestContext.ino),
[keep-alive](examples/KeepAlive/KeepAlive.ino), and
[static files](examples/FileServer/FileServer.ino) have separate examples for
boards with more memory.

## Migrating from 3.x

- Replace `Application` with `App`.
- Replace `app.header(...)` with a per-request `Request::Header` array passed to
  `process(client, headers)`. Use `App::ProcessOptions` to combine headers, custom
  buffers, and context; `process(client, context)` becomes
  `options.context = context` followed by `process(client, options)`.
- Call `req.next()` when middleware should continue to the next matching
  handler. Returning from a handler alone stops dispatch.
- Mount a router with `app.use("/prefix", &router)`. Routers and applications
  cannot be copied; mounted routers must outlive the application using them.
- Use `WsChannel` and `WsMessage` for WebSockets. Include `<WsChannel.h>`
  explicitly. The [WebSockets example](examples/WebSockets/WebSockets.ino) shows header
  capture, upgrades, callbacks, and rooms.
- Use `SseChannel` from `<SseChannel.h>` for server-sent events. The
  [SSE example](examples/ServerSentEvents/ServerSentEvents.ino) shows how to retain
  the client after opening the response.

## Routes and middleware

```cpp
Router cats;

void logRequest(Request& req, Response& res) {
  Serial.println(req.path());
  req.next();
}

void cat(Request& req, Response& res) {
  char id[32];
  if (!req.route("id", id, sizeof(id))) return res.sendStatus(400);
  res.print(id);
}

void registerRoutes() {
  app.use(logRequest);
  cats.get("/:id", cat);
  app.use("/cats", &cats);
}
```

`get`, `head`, `post`, `put`, `patch`, `del`, and `options` register method-specific
handlers. `use` registers middleware or mounts another router. `notFound`
customizes the fallback response, and `finally` runs after dispatch. A middleware
that rejects a request should send its response and return without `req.next()`.

## Request parameters and headers

`req.path()` contains the decoded path; `req.query()` contains the raw query.
Named query parameters and form fields are decoded into caller-owned buffers.
Buffer capacities include the terminating NUL; check the returned boolean for
missing, malformed, or oversized values.

```cpp
void search(Request& req, Response& res) {
  char term[64];
  if (!req.query("q", term, sizeof(term))) return res.sendStatus(400);
  res.print(term);
}
```

Capture only the headers a handler needs. Buffer capacities and the number of
headers are inferred from the arrays:

```cpp
void handleClient(Client& client) {
  char userAgent[128];
  Request::Header headers[] = {{"User-Agent", userAgent}};
  app.process(&client, headers);
}

void showAgent(Request& req, Response& res) {
  const char* agent = req.get("User-Agent");
  res.print(agent ? agent : "Unknown");
}
```

`req.get(name)` returns `nullptr` if the header is absent or was not selected for
capture. A present header with an empty value returns an empty string. The
header array, names, and value buffers must remain alive throughout `process()`.
Each concurrent invocation needs its own header array and buffers; header values
are cleared before parsing and after processing.

Form bodies can be read field by field with
`req.form(name, sizeof(name), value, sizeof(value))`, or through `req.read()`,
`req.readBytes()`, and other `Stream` methods. `req.left()` reports the remaining
Content-Length. Incoming chunked request bodies are unsupported and rejected.

## Responses and connections

Set response headers before writing the body. `res.sendStatus(code)` sends the
standard status text when the status permits a body. `res.status(code)` selects
a status for a custom response. Header and body writes are finalized when
`process()` returns.

`App::ProcessResult` describes connection ownership:

- `responseOpen`: the handler called `res.keepOpen()`, or upgraded the connection.
  Retain the transport for the channel to use after `process()`.
- `keepAlive`: the HTTP response and request body completed successfully and the
  connection can process another request. The caller still owns the client and
  its idle timeout.
- Otherwise, stop the client.

For SSE, call `events.open(res)` in the route, then `events.add(client)` when
`process()` returns `responseOpen`. WebSocket `upgrade(req, res)` retains a copy
of the underlying client itself. Channel client types must match the transport
passed to `process()` and support copying the connection handle. Call
`sockets.poll()` regularly to process incoming frames and heartbeats.

## Memory and build configuration

Uno automatically uses a 64-byte URL buffer and 32-byte output and pushback
buffers. Route nodes are allocated when routes are registered. Keep route tables
and channel capacities small, and store fixed response strings in flash:

```cpp
P(page) = "<html><body>Hello</body></html>";
res.printP(page);
```

`ProcessOptions` combines headers, custom buffers, and context for an individual
request. Assign arrays directly to infer their sizes:

```cpp
void handleClient(Client& client, void* context) {
  char authorization[64];
  char url[128];
  uint8_t output[256];
  Request::Header headers[] = {{"Authorization", authorization}};

  App::ProcessOptions options;
  options.headers = headers;
  options.urlBuffer = url;
  options.writeBuffer = output;
  options.context = context;
  app.process(&client, options);
}
```

Options reference caller-owned storage, which must remain alive throughout
`process()`. Omitted buffers or null buffer pointers select defaults. A URL
buffer needs at least two bytes; an output buffer must be larger than
`CHUNK_OVERHEAD`. When using raw pointers, provide capacities explicitly, for
example `options.urlBuffer = {urlPointer, urlCapacity}` or
`options.headers = {headerPointer, headerCount}`. A header with a raw buffer
pointer is constructed as `Request::Header{"Name", valuePointer, valueCapacity}`.

Code using the earlier 4.0 options API should replace `Request::HeaderNode` with
`Request::Header`, remove header `next` pointers, and remove the separate
`headerCount`, `urlBufferLength`, and `writeBufferLength` option assignments.
Use arrays directly or pair raw pointers with their sizes as above.

Advanced builds can override `SERVER_URL_BUFFER_SIZE`,
`SERVER_OUTPUT_BUFFER_SIZE`, `SERVER_PUSHBACK_BUFFER_SIZE`, or `LOW_MEMORY_MCU`
with compiler flags applied to **all library and sketch translation units**.
Defining these only in a sketch cannot change the separately compiled library.
The same rule applies to `AWOT_NO_STD_FUNCTION_MIDDLEWARE`, which selects function
pointers instead of automatic `std::function` support. Member-function binding
and capturing lambdas require the latter.

WebSocket channels default to 256-byte incoming frames. A sketch can set
`WS_RX_BUFFER_SIZE` before including `<WsChannel.h>`; use the same definition in
all translation units that instantiate that channel. Frames must be masked and
unfragmented. Oversized frames are closed with code 1009. At most 32 rooms are
supported. Room names must remain alive for the channel's lifetime.

See [source contracts](src/README.md) for buffer boundaries, parsing behavior,
and transport failure handling.

## Development

Development files are available in a Git checkout under `.extras/` and are
excluded from library releases. The [macOS native example](.extras/MacOSNative/README.md)
runs the server without Arduino hardware; build and test it with
`make -C .extras/MacOSNative test`.

The native C++11 tests use the Arduino stubs in `.extras/test/epoxy`:

```sh
cmake -S .extras/test -B /tmp/awot-tests
cmake --build /tmp/awot-tests --parallel
ctest --test-dir /tmp/awot-tests --output-on-failure
```

Run with AddressSanitizer and UndefinedBehaviorSanitizer:

```sh
cmake -S .extras/test -B /tmp/awot-sanitized -DAWOT_ENABLE_SANITIZERS=ON
cmake --build /tmp/awot-sanitized --parallel
ctest --test-dir /tmp/awot-sanitized --output-on-failure
```

The smaller configuration also tests function-pointer callbacks consistently
across the library and test translation units:

```sh
cmake -S .extras/test -B /tmp/awot-small -DAWOT_ENABLE_SANITIZERS=ON \
  -DAWOT_NO_STD_FUNCTION_MIDDLEWARE=ON -DAWOT_SMALL_BUFFERS=ON
cmake --build /tmp/awot-small --parallel
ctest --test-dir /tmp/awot-small --output-on-failure
```

Compile the Uno examples with the Arduino CLI:

```sh
arduino-cli core install arduino:avr
arduino-cli lib install Ethernet
arduino-cli compile --fqbn arduino:avr:uno --library . examples/Ethernet
arduino-cli compile --fqbn arduino:avr:uno --library . examples/SerialBridge
```

CI runs native tests on Linux and macOS, sanitizer and small-buffer configurations
on Linux, and example builds for Uno, Leonardo, Mega, Due, and ESP32. Native tests
do not measure on-device stack usage or network timing.

## Library releases

Keep the versions in `library.properties` and `library.json` identical before
creating a release tag. Arduino Library Manager packages the tagged repository
and excludes dot-prefixed paths, including `.extras/`; it does not honor
`.gitattributes`. Git source archives use `.gitattributes` to omit those same
paths. PlatformIO's `export` settings include `src/`, Arduino `examples/`, and
`keywords.txt`, alongside the manifests, README, and license that PlatformIO
always includes.

Check the metadata and inspect a local PlatformIO package before publishing:

```sh
arduino-lint --library-manager update --compliance strict "$PWD"
pio pkg pack --output /tmp/aWOT.tar.gz
tar -tzf /tmp/aWOT.tar.gz
```

## Acknowledgements and license

Based on Webduino, Copyright 2009–2014 Ben Combee, Ran Talbott, Christopher Lee,
Martin Lormes, Francisco M Cuenca-Acuna. Licensed under [MIT](LICENSE).
