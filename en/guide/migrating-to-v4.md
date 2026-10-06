---
layout: page
title: Migrating to aWOT 4
menu: guide
lang: en
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

```arduino
// 4.x
App app;
```

Apps and routers cannot be copied. Register routes on the existing instance
and mount routers by pointer. The v4 library has multiple source and header
files; install the whole library instead of copying the old `aWOT.h` and
`aWOT.cpp` pair into a sketch.

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

```arduino
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

```arduino
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

```arduino
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

<h2 id="client">Use client() instead of stream()</h2>

```cpp
// 3.x: inside a handler
Stream* transport = req.stream();
```

```arduino
// 4.x: inside a handler
Client* transport = req.client();
```

The Client overload exposes your supplied client. `process(Stream*)` creates
a temporary `StreamClient` adapter: do not retain its pointer after processing.
`Request` and `Response` also exist only during the call. Long-lived connections
must retain concrete client handles owned by the caller.

<h2 id="router">Mount routers explicitly</h2>

The supported mounting pattern remains explicit:

```arduino
// File scope
Router api;
```

```arduino
// Inside setup(), after adding routes to api
app.use("/api", &api);
```

Keep the router alive while it is mounted. Apps and routers own their route
nodes, but borrow mounted routers, path strings, and callback instances.
Mounts now require a path segment boundary: `/api` will not mount on `/apiculture`.
Trailing slashes on mount prefixes are ignored; nested mounts accumulate
prefixes. Callback `use(path, handler)` still matches a complete route pattern,
not a prefix. See [routing](/en/guide/routing.html).

Older documentation and drafts showed `Router("/prefix")`. That is not the
mounting API; the v3 release headers use `Router()` and the discarded constructor
path in older drafts did not mount routes. Use `Router router;` and
`app.use("/prefix", &router)` in v4. Older guide calls to `app.all()` and
`res.success()` were documentation errors, rather than APIs removed in v4:
use `app.use()` and `res.set("Content-Type", "text/html")` with normal response
output.

<h2 id="connections">Check the processing result</h2>

`process()` now returns `App::ProcessResult` instead of `void`. For the common
close-after-response case, keep an explicit `client.stop()` after processing.
For an application already managing retained clients, use the result:

```arduino
// Inside a caller that retains this concrete client handle
App::ProcessResult result = app.process(&client);
if (!result.keepAlive && !result.responseOpen) {
  client.stop();
}
```

`keepAlive` requires an explicit `Connection: keep-alive` response header,
a consumed request body, successful parsing and writes, and no open response.
HTTP/1.1 alone does not enable reuse. The caller implements idle timeouts and
closes rejected or expired clients.

`res.keepOpen()` leaves an HTTP stream or protocol upgrade open after
processing. Call it before emitting ordinary HTTP headers, then retain the
client only if `responseOpen` is true. Ordinary open responses are
close-delimited; aWOT removes length/chunk framing and sets Connection to close.
A 101 upgrade retains its upgrade headers. Follow the complete
[connection examples](/en/guide/connections.html),
[WebSocket guide](/en/guide/websockets.html), and
[SSE guide](/en/guide/server-sent-events.html) for ownership and cleanup.

<h2 id="parsing-and-errors">Parsing, responses, and errors</h2>

Recheck applications that depended on permissive parsing. `path()` is decoded
and `query()` is raw; named query/form decoding treats `+` as a space, while
path decoding preserves `+`. Invalid target escapes or decoded NUL produce
400. Query/form capacities include NUL, and extraction can fail; always check
the returned boolean. Incoming Transfer-Encoding, including chunked bodies,
is rejected. Repeated Content-Length values must agree and fit `INT_MAX`.

`app.onError(handler)` adds a shared parser and missing-route error callback.
A custom error response must call `res.end()` to suppress the built-in fallback.
`finally` runs before response finalization, including after parser errors;
invalid process options bypass it. `setTimeout()` and `minorVersion()` already
existed in v3; the current guides now document their behavior.

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
