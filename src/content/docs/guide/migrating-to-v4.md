---
title: "Migrating to aWOT 4"
description: "Update aWOT 3 projects for the API, middleware, headers, and connection behavior in aWOT 4."
route: "/en/guide/migrating-to-v4.html"
section: "guide"
---

# Migrating to aWOT 4

Install **aWOT 4.0.0** through Arduino Library Manager and update sketches using
the changes below. The [3.x API](/en/3x/api.html) remains available for existing
applications; the [current reference](/en/4x/api.html) documents v4. The snippets
on this page are partial before-and-after examples. Replace the indicated
declarations, handlers, or call sites in your existing sketch.

<h2 id="app">Application becomes App</h2>


Keep including `<aWOT.h>`, but rename the application class:

```cpp
// 3.x
Application app;
```

```cpp
// 4.x
App app;
```

If a helper function takes an app or router by value, change it to a reference:
for example, `void addRoutes(App& app)` or `void addRoutes(Router& router)`.
aWOT 4 rejects copies of these objects because each instance owns its registered
routes. Sketches that register routes directly on a global app are unaffected.

The v4 library has multiple source and header files; install the whole library
instead of copying the old `aWOT.h` and `aWOT.cpp` pair into a sketch.

<h2 id="continuation">Middleware continues explicitly</h2>


In 3.x, returning from a handler allowed the next matching registration to run
unless the response was ended. In v4, returning stops dispatch; call
`req.next()` when you want to continue:

```cpp
// 3.x logger: returning continued dispatch.
void logRequest(Request& req, Response& res) {
  Serial.println(req.path());
}
```

```cpp
// 4.x logger
void logRequest(Request& req, Response& res) {
  Serial.println(req.path());
  req.next();
}
```

Use this change in authentication middleware, multiple registrations for one
route, and handlers inside mounted routers. Keep terminal error paths without
`req.next()`. `res.end()` still marks a response as ended and prevents further
route dispatch. The final callback still runs before finalization.

<h2 id="headers">Header capture belongs to process()</h2>


Replace application-wide header registration:

```cpp
// 3.x: declaration and setup registration
char authorization[64];
// Inside setup():
app.header("Authorization", authorization, sizeof(authorization));
```

with caller-owned capture storage for each processing call:

```cpp
// 4.x: inside the connected-client branch of loop()
char authorization[64];
Request::Header headers[] = { {"Authorization", authorization} };
app.process(&client, headers);
client.stop();
```

The array is contiguous; capacities and entry counts are inferred. Alternatively
assign it to `ProcessOptions.headers`. Raw buffers need
`Request::Header{"Authorization", pointer, capacity}`. Names, capture entries,
and buffers must live through `process()` and be separate for concurrent calls.
Values are cleared before and after the call.

In a handler, `req.get(name)` now returns null for an absent or uncaptured
header. A present empty header returns a non-null empty string. Check for null
before comparing or dereferencing it. Copy values that you need after processing.

<h2 id="process-options">Context and buffers use ProcessOptions</h2>


Replace positional context and buffer arguments with options. This example
assumes `client` and application context `state` already exist in the caller:

```cpp
// 3.x
char url[64];
uint8_t output[32];
app.process(&client, url, sizeof(url), output, sizeof(output), &state);
```

```cpp
// 4.x
char url[64];
uint8_t output[32];
App::ProcessOptions options;
options.urlBuffer = url;
options.writeBuffer = output;
options.context = &state;
app.process(&client, options);
client.stop();
```

`req.context` still exposes the supplied pointer. The new `ArrayView` fields
borrow storage: arrays infer sizes; raw pointers require `{pointer, size}`.
There are no separate option length/count fields or linked `HeaderNode` entries.
Null URL/output pointers select defaults. Supplied URL storage needs at least
two bytes; output storage must exceed `CHUNK_OVERHEAD`. Invalid options stop the
client without invoking routes, `onError`, or `finally`.

<h2 id="client">Request::stream() becomes Request::client()</h2>


In v3, `req.stream()` returned a `Stream*` for the underlying connection.
In v4, this method is replaced by `req.client()`, which returns a `Client*`.
If your handler calls `req.stream()`, update it as follows:

```cpp
// 3.x: inside a handler
Stream* transport = req.stream();
```

```cpp
// 4.x: inside a handler
Client* transport = req.client();
```

<h2 id="router">Router mount matching</h2>


Router mounts now match complete path segments: a router mounted at `/api`
receives `/api` and `/api/users`, but no longer `/apiculture`. Trailing slashes
on mount paths are ignored, so `/api/` and `/api` now behave the same.
Check any mounts that relied on partial names or trailing slashes. See
[routing](/en/guide/routing.html) for examples.

<h2 id="connections">Check the processing result</h2>


`process()` now returns `App::ProcessResult` instead of `void`. For the common
close-after-response case, keep an explicit `client.stop()` after processing.
For an application already managing retained clients, use the result:

```cpp
// client is stored outside loop(), or in a persistent client pool.
App::ProcessResult result = app.process(&client);
if (!result.keepAlive && !result.responseOpen) {
  client.stop();
}
```

`keepAlive` requires an explicit `Connection: keep-alive` response header,
a consumed request body, successful parsing and writes, and no open response.
HTTP/1.1 alone does not enable reuse. The caller implements idle timeouts and
closes rejected or expired clients.

The result does not retain the transport. ESP32 closes the socket when the last
`WiFiClient` owner is destroyed, so a local client inside `loop()` still closes
at scope exit even when `keepAlive` is true. Keep the handle at file scope or
in persistent storage, as shown in the complete
[connection reuse sketch](/en/guide/connections.html#keep-alive).

`res.keepOpen()` leaves an HTTP stream or protocol upgrade open after
processing. Call it before emitting ordinary HTTP headers, then retain the
client only if `responseOpen` is true. Ordinary open responses are
close-delimited; aWOT removes length/chunk framing and sets Connection to close.
A 101 upgrade retains its upgrade headers. Follow the complete
[connection examples](/en/guide/connections.html),
[WebSocket guide](/en/guide/websockets.html), and
[Server Sent Events guide](/en/guide/server-sent-events.html) for ownership and cleanup.

<h2 id="parsing-and-errors">Parsing, responses, and errors</h2>


Recheck applications that depended on permissive parsing. `path()` is decoded
and `query()` is raw; named query/form decoding treats `+` as a space, while
path decoding preserves `+`. Invalid target escapes or decoded NUL produce
400. Query/form capacities include NUL, and extraction can fail; always check
the returned boolean. HTTP/1.1 requests support a single
`Transfer-Encoding: chunked` field.
Repeated or unsupported transfer codings, transfer coding on HTTP/1.0, and
Content-Length combined with Transfer-Encoding are rejected with 400.
Repeated Content-Length values must agree and fit `INT_MAX`. Chunked reads
decode payload directly; extensions are ignored and trailers are discarded.
Each chunk size can be up to `UINT32_MAX`.

For chunked requests, `req.left()` returns `-1` until the final chunk and
trailers have been consumed, then the remaining pushback count or zero. Change
generic body loops from `req.left() > 0` to `req.left() != 0`, and handle failed
reads. Code that requires a known total length can continue rejecting negative
`left()` values. An incomplete chunked body allows neither connection reuse nor
an open-response handoff. Malformed chunk framing closes the connection and,
if no response status was sent when routing finishes, invokes `onError` with
400; an existing response is not replaced.

`app.onError(handler)` adds a shared parser and missing-route error callback.
A custom error response must call `res.end()` to suppress the built-in fallback.
`finally` runs before response finalization, including after parser errors;
invalid process options bypass it.

Positive short writes are retried. A zero transport write closes the client and
sets `getWriteError()`; buffered acceptance is not proof of delivery.
`bytesSent()` counts accepted response bytes, including headers. HEAD requires
an explicit route and suppresses its payload; bodyless statuses also suppress
output. Interim responses reset state for a final response. `setDefaults()` is
retained as a compatibility no-op; defaults are chosen when headers are emitted.

<h2 id="callbacks">Callback and configuration changes</h2>


Supported platforms now select `std::function` automatically. They accept
capturing lambdas and member callbacks such as
`app.get("/count", &counter, &Counter::handle)`. Keep the instance alive while
registered. Function-pointer builds accept ordinary functions and noncapturing
lambdas. Apply `AWOT_NO_STD_FUNCTION_MIDDLEWARE` and HTTP layout overrides to
every library and sketch translation unit, rather than defining them only in
a sketch. See [configuration](/en/guide/configuration.html).

<h2 id="prerelease-sse">Earlier pre-release v4 SSE sketches</h2>


The following changes apply to sketches using the earlier pre-release v4 SSE
helper. Sending now follows the same recipient selection as WebSockets:

| Earlier v4 SSE call | Updated call |
| --- | --- |
| `events.send("data")` | `events.broadcast("data")` |
| `events.send("sample", "data")` | `events.broadcast("sample", "data")` |

Use `events.send(clientId, data)` or `events.send(clientId, event, data)` to
send to one active client. `events.to(room, data)` and
`events.to(room, event, data)` target room members; `room` can be an integer
index or a registered name. Broadcast calls still send to every active client.

Change connection callbacks from `void callback(Print&)` to
`void callback(int clientId)`. The callback can join a room or send an initial
event with `events.join(clientId, "chat")` and
`events.send(clientId, "ready", "connected")`. Register the room with
`events.defineRoom("chat")` before accepting clients. `forEach` callbacks
continue to receive `Print&` for comments or custom fields.

`add(client)` now returns the assigned client slot ID, or `-1` when the channel
is full and stops the supplied client. `count()` and `connected(clientId)`
report active slots; `close(clientId)` stops one client. IDs are reused, and
new clients start with no room memberships. Sending and `forEach()` detect
disconnected clients and clean up their slots. SSE has no `poll()` or
`onDisconnect` callback.

`SseChannel<TClient, N, MAX_ROOMS>` defaults to 8 rooms, so existing declarations
with only a client type and capacity remain valid. Both channels support
`defineRoom()`, `findRoom()`, `roomName()`, `join()`, `leave()`, and `inRoom()`
with the same room behavior: case-sensitive names, duplicate registration
returning the existing index, and at most 32 rooms. Names are borrowed and must
remain valid and unchanged for the channel's lifetime. See the
[SSE guide](/en/guide/server-sent-events.html) for the complete connection flow.
