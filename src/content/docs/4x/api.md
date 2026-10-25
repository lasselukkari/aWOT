---
title: "aWOT 4.x - API Reference"
description: "The aWOT 4 API reference for App, Router, Request, Response, WebSockets, server-sent events, and configuration."
route: "/en/4x/api.html"
section: "api"
apiVersion: "4x"
routeAlias: "/en/api.html"
canonical: "/en/4x/api.html"
---

# aWOT 4 API

This reference targets **aWOT 4.0.0**. Install it through Arduino Library Manager.
For existing projects, read [Migrating to v4](/en/guide/migrating-to-v4.html).
The [3.x reference](/en/3x/api.html) remains available for older installations.

<h2 id="app">App</h2>


`App` registers routes and synchronously processes one HTTP request at a time.
Include `<aWOT.h>`. The examples below use its default namespace imports; when
`ENABLE_AWOT_NAMESPACE` is defined, qualify types with `awot::`.

```cpp
App app;

void home(Request& req, Response& res) {
  res.print("Hello World!");
}

// Register once in setup(): app.get("/", home);
// With a connected Client in loop():
// app.process(&client);
// client.stop();
```

<h3 id="app-App">Construction and ownership</h3>


```cpp
App();
~App();
```

Apps cannot be copied or assigned: their owned default `Router` is noncopyable.
Destroying an App releases its route registrations, but does not destroy mounted
routers, callback instances, or network clients. Keep registered path strings,
mounted routers, and bound callback instances alive while the App uses them.
Registration methods return `void`; they provide no allocation-failure result.

<span id="app-methods"></span>
<h3 id="app-get">App::get</h3>


```cpp
void get(const char* path, Router::MIDDLEWARE_PARAM middleware);
void get(Router::MIDDLEWARE_PARAM middleware);
```

Register a GET handler. The overload without a path matches every GET path.
Handlers run in registration order. Call [`req.next()`](#req-next) to continue;
returning without it stops dispatch. See [route matching](#router-use) and
[`Router::Middleware`](#router-Middleware) for callback configuration.

<h3 id="app-head">App::head</h3>


```cpp
void head(const char* path, Router::MIDDLEWARE_PARAM middleware);
void head(Router::MIDDLEWARE_PARAM middleware);
```

Register a HEAD handler. HEAD needs its own route; a GET registration does not
handle HEAD. Status and headers are sent, while response payload and chunk
terminators are suppressed. The pathless overload matches every HEAD path.

<h3 id="app-post">App::post</h3>


```cpp
void post(const char* path, Router::MIDDLEWARE_PARAM middleware);
void post(Router::MIDDLEWARE_PARAM middleware);
```

Register a POST handler, optionally restricted to a route pattern.

<h3 id="app-put">App::put</h3>


```cpp
void put(const char* path, Router::MIDDLEWARE_PARAM middleware);
void put(Router::MIDDLEWARE_PARAM middleware);
```

Register a PUT handler, optionally restricted to a route pattern.

<h3 id="app-del">App::del</h3>


```cpp
void del(const char* path, Router::MIDDLEWARE_PARAM middleware);
void del(Router::MIDDLEWARE_PARAM middleware);
```

Register a DELETE handler. The C++ method name is `del`.

<h3 id="app-patch">App::patch</h3>


```cpp
void patch(const char* path, Router::MIDDLEWARE_PARAM middleware);
void patch(Router::MIDDLEWARE_PARAM middleware);
```

Register a PATCH handler, optionally restricted to a route pattern.

<h3 id="app-options">App::options</h3>


```cpp
void options(const char* path, Router::MIDDLEWARE_PARAM middleware);
void options(Router::MIDDLEWARE_PARAM middleware);
```

Register an OPTIONS handler. aWOT does not generate an OPTIONS or CORS response
automatically.

<h3 id="app-use">App::use</h3>


```cpp
void use(const char* path, Router* router);
void use(Router* router);
void use(const char* path, Router::MIDDLEWARE_PARAM middleware);
void use(Router::MIDDLEWARE_PARAM middleware);
```

Callback overloads match every supported HTTP method. Their optional path is a
route pattern, so `app.use("/api", handler)` matches `/api` and `/api/`, rather
than every descendant path. A pathless callback matches every path.

Router overloads mount a borrowed Router at a path prefix with a segment
boundary. `app.use("/api", &router)` mounts routes under `/api` and does not
match `/api2`. Mounting without a path preserves the whole path. See
[`Router::use`](#router-use) for trailing slashes and nested mounts.

<h3 id="app-memberCallbacks">Member callbacks</h3>


With `STD_FUNCTION_MIDDLEWARE`, each method-specific registration and `use`
also provides these exact template overloads:

```cpp
template <typename T>
void get(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void get(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void head(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void head(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void post(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void post(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void put(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void put(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void del(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void del(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void patch(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void patch(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void options(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void options(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void use(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void use(T* instance, void (T::*method)(Request&, Response&));
```

These bind a non-const member function to the supplied instance. The App stores
the binding, and the instance must outlive its registrations. Capturing lambdas
are also available with this callback configuration. Function-pointer builds
support free functions and noncapturing lambdas. See
[`Router::Middleware`](#router-Middleware).

<h3 id="app-notFound">App::notFound</h3>


```cpp
void notFound(Router::MIDDLEWARE_PARAM middleware);
#ifdef STD_FUNCTION_MIDDLEWARE
template <typename T>
void notFound(T* instance, void (T::*method)(Request&, Response&));
#endif
```

Set the single fallback callback, replacing any earlier one. It runs after
successful parsing if routing leaves both `statusSent()` and `ended()` false,
even if a matched handler returned without writing a response. aWOT sends status
404 before calling it. Supply the headers and body in the callback.

With no custom fallback, aWOT invokes `onError` with 404 and sends its built-in
404 response unless the error callback ends the response. A custom `notFound`
callback takes precedence over that 404 error path.

<h3 id="app-finally">App::finally</h3>


```cpp
void finally(Router::MIDDLEWARE_PARAM middleware);
#ifdef STD_FUNCTION_MIDDLEWARE
template <typename T>
void finally(T* instance, void (T::*method)(Request&, Response&));
#endif
```

Set the single final callback, replacing any earlier one. It runs once after
routing, fallback, or a parser error and before buffered output and the final
chunk are finalized. It runs even after `res.end()`. On parser errors, request
fields may be incomplete. Invalid processing options and null transports bypass
all callbacks, including `finally`.

<h3 id="app-onError">App::onError and ErrorHandler</h3>


```cpp
typedef void (*ErrorHandler)(Request&, Response&, int statusCode);
void onError(ErrorHandler handler);
```

Set the single error callback; `nullptr` removes it. This is always a function
pointer, including builds using `std::function` for ordinary middleware. It
receives parser errors or the default 404 fallback: 400 for malformed input or
unsupported request transfer encoding, 408 for parser read timeout, 414 for an
invalid or oversized request target, 431 for captured-header overflow or certain
header read failures, and 505 for an unsupported HTTP version.

To supply a custom response, call `res.end()` after writing it. Otherwise aWOT
continues with its built-in `sendStatus(statusCode)` response. Parser errors
close the client after finalization and never allow connection reuse.
Malformed chunk framing discovered during body reads also closes the client.
If no response status has been sent when routing finishes, aWOT invokes
`onError` with 400; an already-started response is not replaced.

```cpp
void errorResponse(Request& req, Response& res, int code) {
  res.status(code);
  res.set("Content-Type", "application/json");
  res.print("{\"error\":true}");
  res.end();
}
// Register once: app.onError(errorResponse);
```

<h3 id="app-setTimeout">App::setTimeout</h3>


```cpp
void setTimeout(unsigned long timeoutMillis);
```

Set the read timeout in milliseconds for future Requests. The default is 1000.
It controls waits for individual input bytes during parsing and initializes the
Request's inherited Stream timeout. It is neither a whole-request deadline nor
a keep-alive idle timeout. Changing a current body's timeout uses
[`req.setTimeout()`](#req-setTimeout).

<h3 id="app-process-options">App::ProcessOptions</h3>


```cpp
struct ProcessOptions {
  ArrayView<char> urlBuffer;
  ArrayView<uint8_t> writeBuffer;
  ArrayView<Request::Header> headers;
  void* context = nullptr;
};
```

All storage is borrowed through `process()`. Arrays infer capacities or entry
counts; raw pointers need `{pointer, size}`. A null URL or write-buffer pointer
selects default storage, ignoring that view's size. A supplied URL buffer needs
at least two bytes, including the terminator. A supplied output buffer needs
more than `CHUNK_OVERHEAD` bytes. Output capacity is additionally limited by the
reserved chunk-size header. Only missing buffers allocate default local arrays.

Header count must be nonnegative; a positive count needs a non-null array, and
each entry needs a non-null name and buffer with positive capacity. Zero count
ignores the header pointer. Invalid options stop the Client and return a result
with both flags false, without invoking handlers. See
[`Request::Header`](#req-header) and [`ArrayView`](#storage-arrayview).

```cpp
char url[64];
uint8_t output[32];
char authorization[96];
Request::Header headers[] = { {"Authorization", authorization} };
int requestId = 7;

App::ProcessOptions options;
options.urlBuffer = url;
options.writeBuffer = output;
options.headers = headers;
options.context = &requestId;
// For raw pointers: options.urlBuffer = {urlPointer, urlCapacity};
// App::ProcessResult result = app.process(&client, options);
```

Keep the options' arrays, header names and value buffers, and context object
alive until `process()` returns. Concurrent requests need separate storage.
Captured header buffers are cleared before and after processing.

<h3 id="app-process-result">App::ProcessResult</h3>


```cpp
struct ProcessResult {
  bool responseOpen = false;
  bool keepAlive = false;
};
```

`responseOpen` means the parsed request successfully opened a response with
[`res.keepOpen()`](#res-keepOpen), and its initial writes succeeded. Retain the
underlying client for the stream or protocol upgrade. A chunked request body
must also be complete, including its final chunk and trailers.

`keepAlive` means parsing succeeded, the request body was completely consumed,
writes succeeded, the response remains ordinary HTTP, and the response selected
`Connection: keep-alive`. HTTP/1.1 alone does not enable reuse. The two flags
cannot both be true. A false flag pair means the caller should close the client.
These flags describe processing, rather than guaranteeing that a peer remains
connected; the caller owns later disconnect handling and idle timeouts.
Neither flag retains the client or extends its lifetime. On ESP32, destroying
the last `WiFiClient` handle closes the socket, even if a flag was true. Keep
the handle in persistent storage, or copy it into an owner such as a channel
before the local handle leaves scope.

<h3 id="app-process">App::process</h3>


```cpp
ProcessResult process(Client* client);
ProcessResult process(Client* client, const ProcessOptions& options);
ProcessResult process(Stream* stream);
ProcessResult process(Stream* stream, const ProcessOptions& options);
template <size_t N>
ProcessResult process(Client* client, Request::Header (&headers)[N]);
template <size_t N>
ProcessResult process(Stream* stream, Request::Header (&headers)[N]);
```

Process one request synchronously, invoke matching handlers, run the final
callback, and finalize buffered output. The header-array overloads infer the
entry count and otherwise use default options. A null transport returns both
flags false. The ordinary success path leaves transport closure to the caller;
invalid options, parser errors, and failed response writes stop a Client.

```cpp
// client is a connected handle stored outside loop(), or in a persistent pool.
App::ProcessResult result = app.process(&client);
if (!result.responseOpen && !result.keepAlive) {
  client.stop();
}
// For keepAlive, retain this client and process its next request when ready.
// For responseOpen, let the stream/channel own subsequent I/O and closure.
```

This fragment assumes that `client` remains alive after the enclosing function
returns. Declaring a fresh local `WiFiClient` inside `loop()` and omitting
`stop()` does not preserve an ESP32 connection. The complete
[connection reuse example](/en/guide/connections.html#keep-alive) shows the
persistent declaration, acceptance, subsequent requests, and idle timeout.

The Stream overload wraps the borrowed Stream in a temporary `StreamClient`.
`req.client()` refers to that adapter only during processing; do not retain it
or pass it to a long-lived channel. The adapter's `stop()` is a no-op and it
reports itself connected, so the caller must manage the actual Stream lifecycle.
Use a real concrete Client for WebSockets and SSE.

<h3 id="app-strcmpi">App::strcmpi and App::strcmpiP</h3>


```cpp
static int strcmpi(const char* s1, const char* s2);
static int strcmpiP(const char* s1, const unsigned char* s2);
```

Compare two NUL-terminated strings ignoring case. Return 0 for equality, -1
when the first sorts earlier, or 1 when it sorts later. `strcmpiP` reads its
second argument from program memory. Supply valid non-null pointers.

<h3 id="app-header">Migrating App header registration</h3>


The old `Application::header()` registration is retired. Capture headers with a
[`Request::Header` array](#req-header) passed to `process(client, headers)` or
assigned to `ProcessOptions.headers`. See the
[v4 migration guide](/en/guide/migrating-to-v4.html).

<h2 id="router">Router</h2>


`Router` groups handlers for mounting in an App or another Router. Registration
and matching follow the same rules as App routes. Include `<aWOT.h>`.

<h3 id="router-Router">Construction and ownership</h3>


```cpp
Router();
Router(const Router&) = delete;
Router& operator=(const Router&) = delete;
~Router();
```

Construct an empty Router. It owns its registration nodes and releases them on
destruction. It borrows mounted routers, path strings, and member-callback
instances; those objects must remain alive while registrations can run. A Router
cannot be copied or assigned. Mount with `app.use("/prefix", &router)`; the old
path-taking constructor is unavailable. Registration methods return `void` and
provide no allocation-failure result. Register routes once, usually in `setup()`.

<span id="router-types"></span>
<h3 id="router-Middleware">Router::Middleware</h3>


The callback type depends on the configuration in every translation unit:

```cpp
#ifdef STD_FUNCTION_MIDDLEWARE
typedef std::function<void(Request& request, Response& response)> Middleware;
#define MIDDLEWARE_PARAM Middleware
#else
typedef void Middleware(Request& request, Response& response);
#define MIDDLEWARE_PARAM Middleware*
#endif
```

The headers use `MIDDLEWARE_PARAM` for the callback parameters shown below.
Selection is automatic on ESP8266, ESP32, or platforms with `<functional>`;
`STD_FUNCTION_MIDDLEWARE` also enables it explicitly. `std::function` supports
capturing lambdas and the member overloads. Function-pointer builds support free
functions and noncapturing lambdas.

For a function-pointer build, apply `AWOT_NO_STD_FUNCTION_MIDDLEWARE` to every
library and sketch translation unit. Defining it only inside a sketch cannot
change compiled library configuration. Defining it together with
`STD_FUNCTION_MIDDLEWARE` is an error. See the
[configuration guide](/en/guide/configuration.html).

A matched callback starts with continuation disabled. `req.next()` enables the
next matching registration; returning alone stops dispatch. `res.end()` stops
dispatch even if `req.next()` was called. Router dispatch never reads or drains
the request body for the callback.

```cpp
void logRequest(Request& req, Response& res) {
  Serial.println(req.path());
  req.next();
}

void showItem(Request& req, Response& res) {
  char id[16];
  if (!req.route("id", id, sizeof(id))) {
    res.sendStatus(400);
    return;
  }
  res.print(id);
}

Router api;
// Register once:
// api.use(logRequest);
// api.get("/items/:id", showItem);
// app.use("/api", &api);
```

<span id="router-methods"></span>
<h3 id="router-get">Router::get</h3>


```cpp
void get(const char* path, MIDDLEWARE_PARAM middleware);
void get(MIDDLEWARE_PARAM middleware);
```

Register GET middleware with a route pattern, or for every GET path when the
path is omitted.

<h3 id="router-head">Router::head</h3>


```cpp
void head(const char* path, MIDDLEWARE_PARAM middleware);
void head(MIDDLEWARE_PARAM middleware);
```

Register HEAD middleware. HEAD never falls back to GET. Response bodies and
chunk terminators are suppressed for HEAD, while status and headers are sent.

<h3 id="router-post">Router::post</h3>


```cpp
void post(const char* path, MIDDLEWARE_PARAM middleware);
void post(MIDDLEWARE_PARAM middleware);
```

Register POST middleware, optionally restricted to a route pattern.

<h3 id="router-put">Router::put</h3>


```cpp
void put(const char* path, MIDDLEWARE_PARAM middleware);
void put(MIDDLEWARE_PARAM middleware);
```

Register PUT middleware, optionally restricted to a route pattern.

<h3 id="router-del">Router::del</h3>


```cpp
void del(const char* path, MIDDLEWARE_PARAM middleware);
void del(MIDDLEWARE_PARAM middleware);
```

Register DELETE middleware. The C++ method is named `del`.

<h3 id="router-patch">Router::patch</h3>


```cpp
void patch(const char* path, MIDDLEWARE_PARAM middleware);
void patch(MIDDLEWARE_PARAM middleware);
```

Register PATCH middleware, optionally restricted to a route pattern.

<h3 id="router-options">Router::options</h3>


```cpp
void options(const char* path, MIDDLEWARE_PARAM middleware);
void options(MIDDLEWARE_PARAM middleware);
```

Register OPTIONS middleware. Generate any required CORS headers in the handler.

<h3 id="router-use">Router::use and matching</h3>


```cpp
void use(const char* path, Router* router);
void use(Router* router);
void use(const char* path, MIDDLEWARE_PARAM middleware);
void use(MIDDLEWARE_PARAM middleware);
```

The callback overload matches every supported HTTP method. A provided callback
path matches the full route, using the same patterns as method-specific
registrations. Patterns are case-sensitive; `:name` consumes a segment and is
read with `req.route("name", buffer, capacity)`. Matching uses the decoded path,
and excludes the query. A route `/items/:id` matches `/items/42` and
`/items/42/`; a single extra trailing slash is accepted after a complete
pattern. A pattern ending in `/` still requires that slash. There are no wildcard
or regular-expression patterns. The pathless overload matches every path.

The Router overload mounts at a segment boundary. Trailing slashes on the mount
prefix are ignored, so `/api`, `/api/`, and `/api///` mount equivalently.
`/api` matches `/api`, `/api/`, and `/api/items`, but excludes `/api2/items`.
The mounted router matches its paths against the suffix after that prefix. Its
`"/"` or `""` root route matches either `/api` or `/api/`. Mounting at `/` or
without a path leaves the full path available.

Nested mounts accumulate their prefixes. For example, a child `/items/:id`
route mounted under `/v1`, inside a Router mounted under `/api`, handles
`/api/v1/items/42`. `req.path()` still returns the full decoded path, while
`req.route()` refers to the current handler's suffix and pattern. Prefixes are
literal strings and do not define route parameters.

Mounted router dispatch shares continuation state with its parent. If a child
callback returns without `req.next()`, parent dispatch stops too. If no child
route matches, or the last child callback calls `req.next()`, subsequent parent
registrations can run. The mount borrows the router rather than copying it.

<h3 id="router-memberCallbacks">Member callbacks</h3>


These overloads exist only with `STD_FUNCTION_MIDDLEWARE`:

```cpp
template <typename T>
void get(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void get(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void head(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void head(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void post(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void post(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void put(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void put(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void del(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void del(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void patch(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void patch(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void options(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void options(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void use(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void use(T* instance, void (T::*method)(Request&, Response&));
```

Pass an instance pointer and a non-const member function. The binding retains
the pointer and the instance must outlive dispatch. For example:

```cpp
class Controller {
 public:
  void index(Request& req, Response& res) { res.print("Controller"); }
};

Controller controller;
Router routes;
// Register once: routes.get("/", &controller, &Controller::index);
```

<h2 id="req">Request</h2>


`Request` extends Arduino `Stream` to read a bounded HTTP body and expose the
parsed request. aWOT constructs it for each `App::process()` call and passes it
to callbacks; its constructor is private. Do not retain a Request, its path or
query pointers, or captured header pointers after processing returns.

<span id="req-types"></span>
<h3 id="req-header">Request::Header</h3>


```cpp
struct Header {
  Header(const char* name, ArrayView<char> buffer);
  Header(const char* name, char* buffer, int bufferLength);
  const char* name;
  ArrayView<char> buffer;
};
```

Describe one header to capture in caller-owned storage. Arrays implicitly
convert to `ArrayView` and infer their capacity. Raw buffers require an explicit
capacity, including the NUL terminator. Put descriptors in one contiguous array:

```cpp
char authorization[96];
char origin[96];
Request::Header headers[] = {
  {"Authorization", authorization},
  {"Origin", origin}
};
// Process a connected client: app.process(&client, headers);
```

`ProcessOptions.headers = headers` also infers the array's entry count. For raw
descriptor pointers, use `{headerPointer, headerCount}`. Names and buffers must
remain valid throughout `process()`; each active descriptor needs a non-null
name and buffer with positive capacity. Buffer contents and presence state are
cleared before parsing and after processing. Use separate storage for concurrent
requests.

Header names match without case sensitivity. Repeated captured fields append
their values with a comma and need capacity for that comma. Leading spaces and
tabs in each value are skipped. Captured-value overflow fails parsing with 431.
The parser handles `Content-Length` and `Transfer-Encoding` internally rather
than populating captures for them. Use `req.left()` to inspect the remaining
body length or completion state. Trailer fields are discarded and do not
populate or change captured headers.

<span id="req-variables"></span>
<h3 id="req-context">Request::context</h3>


```cpp
void* context;
```

The pointer supplied by `ProcessOptions.context`, defaulting to `nullptr`.
aWOT neither owns nor copies the referenced object. Keep it alive through
processing, use the correct type when casting, and check for null when the
context is optional.

```cpp
struct RequestContext { unsigned long started; };

void elapsed(Request& req, Response& res) {
  RequestContext* context = static_cast<RequestContext*>(req.context);
  if (context) res.print(millis() - context->started);
  else res.sendStatus(500);
}
```

<span id="req-methods"></span>
<h3 id="req-client">Request::client</h3>


```cpp
Client* client();
```

Return the borrowed underlying Arduino Client. Reading it directly bypasses
the Request's body bounds, chunk decoding, pushback, and counters. Use `req`
for an ordinary body; direct Client I/O is useful after a protocol upgrade or
when continuing a response opened with `res.keepOpen()`.

For `process(Stream*)`, the pointer identifies a temporary `StreamClient`
adapter that expires when `process()` returns. Do not retain it. Channels
require the correct concrete Client type, rather than that temporary adapter.

<h3 id="req-methodType">Request::MethodType</h3>


```cpp
enum MethodType {
  UNKNOWN, GET, HEAD, POST, PUT, DELETE, PATCH, OPTIONS, ALL
};
```

`UNKNOWN` is the initial value; the seven supported request verbs set their
corresponding value. `ALL` is the internal registration selector used by `use`.
It is not an incoming HTTP verb. Unsupported verbs fail parsing with 400.

<h3 id="req-method">Request::method</h3>


```cpp
MethodType method();
```

Return the parsed HTTP method. Parsing-error callbacks may see `UNKNOWN`.

<h3 id="req-minorVersion">Request::minorVersion</h3>


```cpp
int minorVersion();
```

Return 0 for HTTP/1.0, 1 for HTTP/1.1, or -1 before a valid version was parsed.
Other versions fail with 505. The version alone does not select keep-alive.

<h3 id="req-next">Request::next</h3>


```cpp
void next();
```

Enable continuation to the next matching registration after this callback
returns. The dispatcher disables continuation before invoking each handler.
Returning without `next()` stops routing. `res.end()` also stops routing and
takes precedence over continuation. Calling `next()` does not recursively run
the next handler or end the response.

<h3 id="req-get">Request::get</h3>


```cpp
char* get(const char* name);
```

Look up a captured header by a valid, NUL-terminated name without case
sensitivity. Return `nullptr` for an absent or uncaptured header. A present
empty value returns a non-null pointer to `""`. The pointer refers to borrowed
capture storage available only during processing.

```cpp
void authenticate(Request& req, Response& res) {
  const char* token = req.get("Authorization");
  if (!token || strcmp(token, "Bearer example") != 0) {
    res.sendStatus(401);
    res.end();
    return;
  }
  req.next();
}
```

Capture `Authorization` with a [`Request::Header`](#req-header) before using
this handler; registering middleware alone does not capture headers.

<h3 id="req-path">Request::path</h3>


```cpp
char* path();
```

Return the NUL-terminated, percent-decoded path without its query. A literal
`+` remains `+`. The parser finds the raw `?` separator before decoding, so an
encoded `%3F` remains part of the decoded path. Invalid percent escapes or
decoded NUL anywhere in the target, including its query, fail parsing with 400.
Mounts do not change this full path. Storage belongs to the active processing
call; use the returned pointer as a read-only value.

<h3 id="req-query">Request::query</h3>


```cpp
char* query();
bool query(const char* name, char* buffer, int bufferLength);
```

The no-argument overload returns the raw encoded query without `?`. After
successful target parsing, a target with no query yields an empty string.
The pointer uses the same borrowed URL storage as `path()`.

The named overload matches decoded names and copies the first matching decoded
value. Both name and value decoding treat `+` as a space. Encoded delimiters
do not split fields. A matching `name=` succeeds with an empty value; a bare
`name` without `=` is not a value. Capacity includes the NUL terminator.

Return `false` for a missing name, invalid input or escape, decoded NUL, or
insufficient capacity. Invalid pointers and nonpositive capacity fail without
touching the buffer. Otherwise the buffer is initialized to empty; overflow
leaves a terminated partial result. Always check the returned boolean.

```cpp
void search(Request& req, Response& res) {
  char term[32];
  if (!req.query("q", term, sizeof(term))) {
    res.sendStatus(400);
    return;
  }
  res.print(term);
}
```

<h3 id="req-route">Request::route</h3>


```cpp
bool route(const char* name, char* buffer, int bufferLength);
bool route(int number, char* buffer, int bufferLength);
```

Copy a segment from the current handler's matched route. The named overload
looks up `:name` in the current pattern. The indexed overload counts all path
segments from zero, including literal segments. For `/items/:id`, index 0 is
`items` and index 1 is the ID. In mounted routers, the route excludes accumulated
mount prefixes, while `path()` still includes them.

Return `true` when the requested segment fits, including its NUL terminator.
Return `false` for a missing segment/name, invalid buffer, nonpositive capacity,
or overflow. Do not use the output on failure. The indexed overload clears the
valid destination first and can leave a terminated partial value on overflow.
The value is already path-decoded; `+` stays literal.

```cpp
void item(Request& req, Response& res) {
  char id[16];
  if (!req.route("id", id, sizeof(id))) {
    res.sendStatus(400);
    return;
  }
  res.print(id);
}
// Register once: app.get("/items/:id", item);
```

<h3 id="req-form">Request::form</h3>


```cpp
bool form(char* name, int nameLength, char* value, int valueLength);
```

Consume the next URL-encoded body field and copy its decoded name and value.
This is sequential reading, rather than a lookup by name. Raw `&` separates
fields and the first raw `=` separates name and value. Percent escapes decode
and `+` becomes space. A field without `=` has an empty value. aWOT does not
check Content-Type before decoding; capture and validate it when required.

Both capacities include a NUL terminator. Return `true` for a complete, valid
field that fits; `false` for no field, invalid escapes, decoded NUL, timeout, or
overflow. Invalid pointers and nonpositive capacity leave output untouched.
Overflow leaves terminated partial output and consumes through the field's raw
`&` or end of body, allowing the next call to read a later field. Check `left()`
to distinguish end of body from unread input after a failure.

```cpp
void saveForm(Request& req, Response& res) {
  char name[16];
  char value[32];
  while (req.left() != 0) {
    if (!req.form(name, sizeof(name), value, sizeof(value))) {
      res.sendStatus(400);
      return;
    }
    // Validate and apply this field here.
  }
  res.sendStatus(204);
}
```

<h3 id="req-left">Request::left</h3>


```cpp
int left();
```

For Content-Length bodies, return the unread declared bytes plus pushed-back
bytes. For chunked bodies, return `-1` until the zero chunk and final empty
trailer line have been consumed, even when pushback contains bytes. After that,
return the pushback count or zero. This call can consume already-available
chunk framing without waiting for input; it does not consume payload. A
malformed or incomplete chunked body continues to return `-1`.

aWOT accepts nonnegative decimal Content-Length up to `INT_MAX`, with
surrounding spaces/tabs. Repeated values must agree. HTTP/1.1 also accepts a
single `Transfer-Encoding: chunked` field, with each chunk size bounded by
`UINT32_MAX`; the complete body can span multiple chunks. Chunk extensions are
ignored and trailer fields are consumed and discarded. Comma lists or
conflicting Content-Length values, Content-Length combined with
Transfer-Encoding, repeated or unsupported Transfer-Encoding values, and
transfer coding on HTTP/1.0 produce 400 and close the connection. Without
either framing header, the body length is zero.

An unread or truncated body prevents keep-alive reuse. An incomplete chunked
body also prevents an open-response handoff. aWOT does not automatically drain
unread payload.

<h3 id="req-available">Request::available</h3>


```cpp
int available();
```

Return immediately available payload bytes, capped at the remaining
Content-Length or the current chunk's remaining payload, plus pushback bytes.
The result saturates at `INT_MAX`. This call can consume already-available
chunk framing without waiting for input; it does not count framing as payload
or look ahead through later chunks. Zero can mean the body is complete or that
its next bytes have not arrived; use `left()` to distinguish them.

<h3 id="req-read">Request::read</h3>


```cpp
int read();
int read(uint8_t* buf, size_t size);
```

The byte overload reads pushback first, then the Client. Return 0–255 for a
byte or -1 when no byte is available or the body is exhausted. It does not wait
for more input. Reaching the body's end through this overload sets the inherited
Stream timeout to zero.

The buffer overload returns the number read, which can be zero at body end or
while chunk framing awaits input. An underlying Client read can return -1 when
no byte could be returned. It can read fewer bytes than requested; use `left()`
to distinguish completion from incomplete input.
Both overloads decode chunked bodies without allocating a body
buffer, and the buffer overload can cross chunk boundaries. They stop at the
body boundary and leave a following HTTP request in the transport. Supply valid
storage for the requested buffer size.
For a timed bounded read, use [`readBytes()`](#req-readBytes) and check its count.

<h3 id="req-peek">Request::peek</h3>


```cpp
int peek();
```

Read one byte and push it back, returning that byte or -1. The next Request read
returns it again. A first peek can increase `bytesRead()` because it initially
reads the underlying Client. It preserves the unread payload count for a
Content-Length body. For chunked bodies, it can advance framing; `left()` keeps
its `-1` result until the final chunk and trailers have been consumed.

<h3 id="req-push">Request::push</h3>


```cpp
void push(uint8_t ch);
```

Push a byte onto the Request's LIFO pushback stack. Reading it again does not
increase `bytesRead()`. `available()` includes pushed bytes; `left()` includes
them once it can report a known remaining length, retaining `-1` while chunked
framing is incomplete. The capacity is `SERVER_PUSHBACK_BUFFER_SIZE`; when
full, a push replaces the previous top byte with the new one and has no failure
return.

<h3 id="req-bytesRead">Request::bytesRead</h3>


```cpp
int bytesRead();
```

Return bytes read from the underlying Client by this Request, including the
request line, headers, body payload, chunk framing, and trailers, saturating at
`INT_MAX`. Pushback rereads are not counted again. Calls such as `left()`,
`available()`, and `peek()` can advance chunk framing and increase this count.

<h3 id="req-availableForWrite">Request::availableForWrite</h3>


```cpp
int availableForWrite();
```

Forward to [`Response::availableForWrite()`](#res-availableForWrite).

<h3 id="req-write">Request::write</h3>


```cpp
size_t write(uint8_t data);
size_t write(const uint8_t* buffer, size_t bufferLength);
```

Forward bytes to the associated Response and return its accepted-byte count.
These methods write the HTTP response; they do not modify the request body.
Use `res` for output so its headers and write-error state are explicit. The
Request's own inherited `Print::getWriteError()` does not report the Response's
write errors; inspect `res.getWriteError()`.

<h3 id="req-flush">Request::flush</h3>


```cpp
void flush();
```

Forward to [`Response::flush()`](#res-flush). It flushes output and does not
discard or consume the incoming request body.

<h3 id="req-inherited">Inherited Arduino interfaces</h3>


The board core supplies the inherited
[Arduino Stream API](https://github.com/arduino/ArduinoCore-API/blob/master/api/Stream.h)
and [Print API](https://github.com/arduino/ArduinoCore-API/blob/master/api/Print.h).
Follow its available overloads. Stream helpers read the bounded body; Print
helpers write the associated response.

<h3 id="req-setTimeout">Request::setTimeout</h3>


Inherited [`Stream::setTimeout(unsigned long)`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Stream.h)
sets the current request's timed-read timeout in milliseconds. `getTimeout()`
reads it; `App::setTimeout()` initializes future Requests.

<h3 id="req-find">Request::find</h3>


Inherited [`Stream::find`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Stream.h)
searches the body for a target.

<h3 id="req-findUntil">Request::findUntil</h3>


Inherited [`Stream::findUntil`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Stream.h)
adds a terminator condition.

<h3 id="req-parseInt">Request::parseInt</h3>


Inherited [`Stream::parseInt`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Stream.h)
reads an integer; validate format when zero and timeout must be distinguished.

<h3 id="req-parseFloat">Request::parseFloat</h3>


Inherited [`Stream::parseFloat`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Stream.h)
reads a floating-point value.

<h3 id="req-readBytes">Request::readBytes</h3>


Inherited [`Stream::readBytes`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Stream.h)
reads with a timeout. Check its count; append any required NUL yourself.

<h3 id="req-readBytesUntil">Request::readBytesUntil</h3>


Inherited [`Stream::readBytesUntil`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Stream.h)
adds a delimiter condition; check its count.

<h3 id="req-readString">Request::readString</h3>


Inherited [`Stream::readString`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Stream.h)
allocates an Arduino String.

<h3 id="req-readStringUntil">Request::readStringUntil</h3>


Inherited [`Stream::readStringUntil`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Stream.h)
adds a delimiter condition.

<h3 id="req-print">Request::print</h3>


Inherited [`Print::print`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Print.h)
formats values into the HTTP response. Prefer `res.print()` in handlers.

<h3 id="req-println">Request::println</h3>


Inherited [`Print::println`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Print.h)
formats values into the response and appends CRLF. Prefer `res.println()`.

<h3 id="req-stream">Migrating Request stream access</h3>


The old `Request::stream()` API is retired. Use [`Request::client()`](#req-client),
which returns `Client*`. An adapter created by `process(Stream*)` exists only
during that call. See the [migration guide](/en/guide/migrating-to-v4.html).

<h2 id="res">Response</h2>


`Response` extends Arduino `Print` and writes the HTTP response associated with
one Request. aWOT constructs it privately and lends it to callbacks during
`App::process()`. Do not retain it after processing. Use the retained Client for
later stream or protocol I/O when `ProcessResult.responseOpen` is true.

By default, the first body write sends status 200 and ordinary headers.
Responses default to `Content-Type: text/plain` when that status permits it and
to a close-delimited body with `Connection: close`. Selecting
`Connection: keep-alive` without Content-Length enables automatic chunked
framing; an explicit `Transfer-Encoding: chunked` uses the same framing.
Default headers do not consume the caller's `SERVER_MAX_HEADERS` slots.

<span id="res-methods"></span>
<h3 id="res-status">Response::status</h3>


```cpp
void status(int code);
```

Emit an HTTP/1.1 status line. Set it before the first body write. A final status
can be sent only once; later calls have no effect. This method does not end
dispatch or emit ordinary headers by itself.

`status(100)`, `status(102)`, and `status(103)` emit and flush an interim response,
then reset status and header state so a final response can follow. Headers
queued before the call belong to that interim response and are cleared
afterward. Status 101 is final for a protocol upgrade and retains its explicitly
queued headers; call `keepOpen()` for a continuing upgraded connection.

HEAD and statuses below 200, 204, 205, and 304 suppress body output and final
chunk terminators. Status 204 omits Content-Length and Transfer-Encoding; 205
uses `Content-Length: 0`. Unknown status codes use their numeric code as the
reason phrase; low-memory builds include a smaller reason-phrase table.

<h3 id="res-statusSent">Response::statusSent</h3>


```cpp
int statusSent();
```

Return the current final status code, or zero before one is selected and after
an interim response resets state. It describes response state, rather than
proof of transport delivery.

<h3 id="res-sendStatus">Response::sendStatus</h3>


```cpp
void sendStatus(int code);
```

Send a status line, ordinary headers, and the reason phrase as the response
body when a body is allowed. For example, `sendStatus(404)` sends `Not Found`
on platforms whose reason-phrase table contains it. HEAD and bodyless statuses
suppress that payload. Codes below 200 emit only the status response.

This method does not call `end()` and does not enable middleware continuation.
Call `res.end()` when a custom error callback must suppress the built-in error
response, or when dispatch must stop despite `req.next()`.

<h3 id="res-set">Response::set</h3>


```cpp
void set(const char* name, const char* value);
```

Queue a header before headers are emitted. Names and values are borrowed
NUL-terminated strings, so keep them alive until emission. Use valid non-null
strings without CR or LF. The method stores pointers and does not validate or
copy them. It appends a slot rather than replacing an existing header; avoid
duplicate framing or connection headers.

The call returns no result and is ignored after headers are sent, after a write
error, or when `SERVER_MAX_HEADERS` slots are occupied. Set required headers
before body output or `beginHeaders()`. `Content-Type`, `Content-Length`,
`Transfer-Encoding: chunked`, and `Connection: keep-alive` affect automatic
defaults and framing, with case-insensitive comparisons. A supplied
Content-Length must equal your actual payload length; aWOT does not verify it.
Explicit chunked encoding takes precedence over a queued Content-Length.

```cpp
void json(Request& req, Response& res) {
  res.status(200);
  res.set("Content-Type", "application/json");
  res.set("Content-Length", "11");
  res.print("{\"ok\":true}");
}
```

<h3 id="res-get">Response::get</h3>


```cpp
const char* get(const char* name);
```

Return the first queued header value matching a valid name without case
sensitivity, or `nullptr` if no caller-supplied header matches. Automatically
generated defaults are not stored in these slots. This queries queued values,
rather than parsing the emitted header block; `keepOpen()` can suppress queued
framing headers during emission.

<h3 id="res-setDefaults">Response::setDefaults</h3>


```cpp
void setDefaults();
```

A compatibility no-op in v4. Defaults and framing are derived by
`beginHeaders()` from the eventual status and connection mode. Calling it does
not emit or reserve headers.

<h3 id="res-beginHeaders">Response::beginHeaders</h3>


```cpp
void beginHeaders();
```

Start the header block, choosing status 200 if no final status exists. It writes
queued and default headers but leaves the final blank line open so custom header
lines can be written with Print methods. Call `endHeaders()` afterward. Calls
are ignored when headers are already sent, headers are already being emitted,
or a write error occurred. Usually `set()` and ordinary body writes suffice.

```cpp
void manualHeader(Request& req, Response& res) {
  res.beginHeaders();
  res.print("X-Device: demo\r\n");
  res.endHeaders();
  res.print("Hello");
}
```

<h3 id="res-endHeaders">Response::endHeaders</h3>


```cpp
void endHeaders();
```

Write the blank line ending the header block and flush buffered headers. If the
block has not started, call `beginHeaders()` first. Repeated calls after header
completion have no effect. Body writes perform both operations automatically
unless a manual header block is active.

<h3 id="res-headersSent">Response::headersSent</h3>


```cpp
bool headersSent();
```

Return whether the header block has completed. Inspect `getWriteError()` as
well when successful transport output matters.

<h3 id="res-end">Response::end</h3>


```cpp
void end();
```

Mark dispatch as ended. Routers stop invoking further handlers, and App error
handling suppresses its built-in fallback response. This method does not flush,
close the Client, or emit a terminal chunk immediately. `finally` still runs,
and `process()` performs output finalization afterward. It does not prohibit
later writes by that callback or the final callback.

<h3 id="res-ended">Response::ended</h3>


```cpp
bool ended();
```

Return the dispatch-ended flag. A write failure also sets it. It does not
indicate that the Client has been closed or the final chunk delivered.

<h3 id="res-keepOpen">Response::keepOpen</h3>


```cpp
void keepOpen();
```

Request continued Client I/O after `process()` returns. Call before emitting
ordinary HTTP headers. For status 200 or above, the initial response becomes
close-delimited: it emits `Connection: close` and suppresses queued
Content-Length, Transfer-Encoding, and Connection headers. Later output goes
directly through the retained Client and ends when the caller closes it.

For status 101, upgrade headers and `Connection: Upgrade` are retained.
Neither open HTTP streams nor protocol upgrades emit a terminal HTTP chunk.
`process()` returns `responseOpen` only after successful parsing and writes.
Calling this after ordinary headers were sent cannot repair their framing.
Use the [Server Sent Events](/en/guide/server-sent-events.html) or
[WebSocket](/en/guide/websockets.html) helpers for their protocol framing.

<h3 id="res-write">Response::write</h3>


```cpp
size_t write(uint8_t data);
size_t write(const uint8_t* buffer, size_t bufferLength);
```

Accept binary bytes into the response. The first body write emits pending
status and headers. Return the count accepted, which can be less than requested
on a transport failure. For a bodyless response, payload is suppressed while
the return value still reports the requested payload accepted. Supply valid
storage for a nonzero buffer length. For text, use the inherited `print()`;
the derived `write` declarations expose the two signatures above.

Small writes may be buffered, so an accepted count does not guarantee delivery.
Large writes flush the buffer and may go directly to the Client; chunked output
splits writes into representable chunks. Positive short Client writes are
retried from the accepted offset. A zero write stops immediately, sets the
Response's inherited Print write error, ends dispatch, closes the Client, and
disables `keepAlive` and `responseOpen`. Later flush or finalization can reveal
a failure after a buffered write had reported success.

```cpp
void bytes(Request& req, Response& res) {
  const uint8_t body[] = {0, 1, 2, 3};
  res.set("Content-Type", "application/octet-stream");
  res.set("Content-Length", "4");
  size_t accepted = res.write(body, sizeof(body));
  res.flush();
  if (accepted != sizeof(body) || res.getWriteError()) {
    // The failed transport has been stopped by aWOT.
    return;
  }
}
```

<h3 id="res-availableForWrite">Response::availableForWrite</h3>


```cpp
int availableForWrite();
```

Return free output-buffer capacity, or zero after a write error. Capacity is the
smaller of storage minus `CHUNK_OVERHEAD` and the size representable in the
reserved hexadecimal chunk header. This describes local buffering rather than
the transport's available send capacity.

<h3 id="res-flush">Response::flush</h3>


```cpp
void flush();
```

Write pending buffered bytes and call the Client's `flush()`. Check
`getWriteError()` afterward. It does not end routing, close a successful client,
or emit the final zero chunk. `process()` performs finalization once callbacks
finish; it flushes pending bytes and terminates ordinary chunked responses.

<h3 id="res-bytesSent">Response::bytesSent</h3>


```cpp
int bytesSent();
```

Return the accepted-byte counter, including HTTP status and header bytes as
well as accepted body bytes, saturating at `INT_MAX`. It includes buffered
bytes that might fail during a later flush. Suppressed body bytes and automatic
chunk framing bytes are not counted. This is not a delivered-body-byte counter.

<h3 id="res-printP">Response::printP</h3>


```cpp
void printP(const unsigned char* string);
void printP(const char* string);
```

Print a NUL-terminated string from program memory. It follows ordinary response
header and buffering rules, stops on write failure, and returns no byte count.
Use `getWriteError()` to inspect failure.

```cpp
void flashText(Request& req, Response& res) {
  P(message) = "Hello from program memory";
  res.printP(message);
}
```

<h3 id="res-writeP">Response::writeP</h3>


```cpp
void writeP(const unsigned char* data, size_t length);
```

Write exactly `length` bytes from program memory, including binary zeros. It
uses ordinary framing and buffering, stops on write failure, and returns no
count. Supply valid program-memory storage for the requested length.

<h3 id="res-print">Response::print</h3>


Inherit all supported formatting overloads from the board core's
[`Print::print`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Print.h),
including C strings, Arduino String, numbers, flash strings, and Printable.
Return values follow accepted-write counts; later buffered failures remain
possible.

<h3 id="res-println">Response::println</h3>


Inherit the board core's
[`Print::println`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Print.h)
overloads, which append CRLF to the formatted output.

<h3 id="res-writeError">Inherited write errors</h3>


[`Print::getWriteError()` and `Print::clearWriteError()`](https://github.com/arduino/ArduinoCore-API/blob/master/api/Print.h)
are inherited. `getWriteError()` becomes nonzero on failed Client writes.
Clearing that flag does not reconnect a closed client or restore dispatch and
connection flags. Let the caller close and discard a failed response transport.

<h2 id="ws">WsChannel</h2>


Include `<WsChannel.h>` explicitly, after `<aWOT.h>`. A channel holds a fixed
number of copied transport handles and receives frames when you call `poll()`.
See the [complete WebSocket guide](/en/guide/websockets.html).

<h3 id="ws-constructor">Constructor and capacities</h3>


```cpp
template <typename TClient, int N, int MAX_ROOMS = 8>
class WsChannel;
WsChannel();
```

`TClient` must be the exact concrete client type supplied to `App::process()`;
`upgrade()` casts `req.client()` to that type. It must support default
construction and copying a handle to the same underlying connection. For
example, use `WsChannel<WiFiClient, 4, 1>` for requests delivered as `WiFiClient`.
The stored copy must keep the connection usable after the original handle is
destroyed. ESP32 `WiFiClient` copies share socket ownership, so destroying the
local accepted handle after a successful upgrade leaves the channel's owner
alive; explicitly stopping any copy closes the shared socket.
The bundled native `MacOSClient`, `LinuxClient`, and `WindowsClient` adapters
support the same ownership model. Their copies also share unread receive bytes,
so a frame buffered during the handshake remains available to the channel.
Do not pass the temporary adapter from `process(Stream*)` to such a channel.
Keep the channel alive while it owns connections; close active sockets before
discarding it. A channel is intended for single-threaded use.

`N` must be positive. `MAX_ROOMS` must be in `1..32`; each socket has a 32-bit
room-membership bitmap. Each of the `N` slots allocates a
`WS_RX_BUFFER_SIZE`-byte message buffer and a separate 125-byte control buffer,
even when idle. The default message capacity is 256 bytes per slot, including
all fragments. See [configuration](#configuration-channels).

<h3 id="ws-upgrade">upgrade</h3>


```cpp
bool upgrade(Request& req, Response& res);
```

Returns `true` after sending the 101 handshake, marking the response open,
copying the client into a free slot, and calling `onConnect`. Capture `Upgrade`,
`Connection`, `Sec-WebSocket-Key`, and `Sec-WebSocket-Version` in a
`Request::Header` array passed to `process()`. The request must be GET over
HTTP/1.1 with no unread body; the upgrade value must be `websocket`, Connection
must contain a complete `upgrade` token, the version must be `13`, and the key
must be a valid base64 encoding of 16 bytes. Value matching is case-insensitive
where appropriate. All four captured buffers must fit their values plus NUL.

Returns `false` for an invalid handshake, missing capture, no available slot,
or failed handshake write. It can reclaim a disconnected slot, calling
`onDisconnect`. A write failure may already have closed the transport, so a
replacement response cannot be guaranteed. The caller must retain the connection
only when `App::ProcessResult.responseOpen` is true, and stop other clients.

```cpp
void websocketRoute(Request& req, Response& res) {
  if (!sockets.upgrade(req, res)) res.sendStatus(400);
}
```

<h3 id="ws-poll">poll and frame handling</h3>


```cpp
void poll();
```

Call frequently from `loop()`. It reads available bytes incrementally without
waiting for a whole frame, dispatches completed text/binary messages, replies to
ping and close frames, and checks disconnections and heartbeat timeouts.
Positive short writes are retried; a zero or invalid write deactivates the slot,
stops its client, and calls `onDisconnect`.

Incoming frames must be masked. Text and binary messages can span several
frames: the first carries the message type, later frames use the continuation
opcode, and the final frame sets `FIN = 1`. The channel assembles the complete
message in its receive buffer and calls `onMessage` once. Ping, pong, and close
frames can arrive between fragments without overwriting the message.

Protocol errors, including an unexpected continuation or a new data message
before the previous one finishes, close with 1002. Messages larger than
`WS_RX_BUFFER_SIZE` across all fragments close with 1009. Text is checked for
valid UTF-8 after the final fragment; a character may span fragments. Invalid
text or close reasons close with 1007. Control frames must be unfragmented and
contain at most 125 payload bytes, independently of `WS_RX_BUFFER_SIZE`.
Disconnecting or reusing a slot clears any incomplete message.

Compression extensions and TLS are not supplied by the channel; the example
uses plain WebSockets. Outgoing messages use a single unmasked frame and are
not limited by the receive buffer size. Keep outgoing lengths within the
32-bit frame-length implementation.

The default heartbeat sends an empty ping after 5000 ms and waits for a pong
for up to 10000 ms. There is at most one outstanding heartbeat ping. Timeout
stops the connection without sending a close frame. Long gaps between `poll()`
calls delay all frame processing and heartbeat checks.

<h3 id="ws-rooms">Rooms</h3>


```cpp
int defineRoom(const char* name);
int findRoom(const char* name) const;
const char* roomName(int roomIdx) const;
void join(int socketId, int roomIdx);
void join(int socketId, const char* name);
void leave(int socketId, int roomIdx);
void leave(int socketId, const char* name);
bool inRoom(int socketId, int roomIdx) const;
bool inRoom(int socketId, const char* name) const;
```

`defineRoom()` returns the existing index for an equal name, even when the room
table is full. A new name gets the next index, starting at zero, or `-1` when
`MAX_ROOMS` distinct names have been registered. Null names return `-1` without
consuming capacity; an empty name is valid. Name comparisons are case-sensitive.
The channel borrows each registered name; use a string literal or storage that
remains alive and unchanged for the channel's lifetime.
`findRoom()` returns a registered name's index, or `-1` for a null or unknown
name. It never creates a room.
`roomName()` returns the registered pointer, or null for an unnamed or
out-of-range index.

`join()`, `leave()`, and `inRoom()` accept either a registered name or an index.
Numeric operations accept any room index in `0..MAX_ROOMS-1`, even before it is
named. Invalid socket or room indices and null or unknown names make `join()`
and `leave()` no-ops and make `inRoom()` return false. These membership methods
do not check whether a socket is active; new upgrades and disconnections clear
membership. Room indices and socket slot IDs are local to this channel. Retain
the index from `defineRoom()` or `findRoom()` to avoid looking up a name on each
call.

```cpp
int chatRoom = sockets.defineRoom("chat");
if (chatRoom >= 0) {
  sockets.join(0, "chat");
  sockets.to("chat", "Welcome!");
  // sockets.join(0, chatRoom) uses the same room without a name lookup.
}
```

<h3 id="ws-send">Sending</h3>


```cpp
void send(int socketId, const char* text);
void send(int socketId, const uint8_t* data, size_t n, bool binary = false);
void to(int roomIdx, const char* text);
void to(int roomIdx, const uint8_t* data, size_t n, bool binary = false);
void to(const char* name, const char* text);
void to(const char* name, const uint8_t* data, size_t n, bool binary = false);
void broadcast(const char* text);
void broadcast(const uint8_t* data, size_t n, bool binary = false);
```

`send()` targets one active socket, `to()` targets active members of a room,
and `broadcast()` targets every active socket. They return no delivery result;
failed writes deactivate the affected connection and call `onDisconnect`.
Invalid indices, null or unknown room names, and inactive `send()` targets are
ignored. `to()` never creates a room. String overloads require a non-null
NUL-terminated string and send a text frame. Buffer overloads
send exactly `n` bytes; pass `true` for binary. Supply valid storage until the
method returns. Outgoing text must be valid UTF-8; these methods do not validate
it. A received payload can contain NUL bytes, so use its pointer
and length rather than the string overload.

```cpp
void echo(WsMessage& msg) {
  sockets.send(msg.socketId(), msg.data(), msg.length(), msg.isBinary());
}
```

<h3 id="ws-state">Connection state and closure</h3>


```cpp
int count() const;
bool connected(int socketId) const;
void close(int socketId, uint16_t code = 1000);
```

`count()` counts active slots. `connected()` reports whether an in-range slot
is active; neither method probes the transport. Call `poll()` regularly to
discover disconnects. IDs can be reused after a disconnect, so do not treat
them as durable client identities.

`close()` sends a close frame, then immediately stops and deactivates an active
socket and calls `onDisconnect`. Invalid/inactive IDs are ignored. Accepted
codes are `1000..1014` except 1004, 1005, and 1006, or `3000..4999`; an invalid
code is replaced with 1002. The method does not wait for the peer's close reply.

<h3 id="ws-callbacks">Callbacks</h3>


```cpp
void onConnect(ConnectCb cb);
void onDisconnect(DisconnectCb cb);
void onMessage(MessageCb cb);
```

With automatic `std::function` support, the public callback types are:

```cpp
typedef std::function<void(int socketId)> ConnectCb;
typedef std::function<void(int socketId)> DisconnectCb;
typedef std::function<void(WsMessage& msg)> MessageCb;
```

With function-pointer callbacks they are:

```cpp
typedef void (*ConnectCb)(int socketId);
typedef void (*DisconnectCb)(int socketId);
typedef void (*MessageCb)(WsMessage& msg);
```

Each setter replaces the previous callback; `nullptr` clears it. `onConnect`
runs during a successful upgrade, after the slot becomes active.
`onDisconnect` runs after deactivation and transport closure.
`onMessage` runs synchronously inside `poll()` once per completed text or binary
message, after its final fragment. The [WsMessage](#wsmessage) object and its
data must not be retained after that callback. Captured objects in callbacks
must outlive their use.

<h2 id="wsmessage">WsMessage</h2>


`WsMessage` is a read-only Arduino `Stream` over a completed WebSocket message.
`<WsChannel.h>` includes it, or include `<WsMessage.h>` explicitly. A channel
creates a temporary message for `onMessage`; copy bytes during the callback if
you need them later. The payload is not NUL-terminated.

<h3 id="wsmessage-constructor">Constructor and opcodes</h3>


```cpp
enum Opcode { TEXT = 0x1, BINARY = 0x2 };
WsMessage(uint8_t* buffer, size_t length, uint8_t opcode, int socketId);
```

The constructor borrows `buffer`, starts the read position at zero, and records
the supplied metadata. It does not validate the pointer or opcode. The buffer
must contain at least `length` readable bytes and outlive this message. Channel
callbacks receive only complete, validated text or binary messages.

<h3 id="wsmessage-read">Reading and Stream methods</h3>


```cpp
int available();
int read();
int peek();
size_t write(uint8_t);
void flush();
```

`available()` returns unread payload bytes. `read()` returns and consumes the
next unsigned byte as an `int`, or `-1` at the end. `peek()` returns it without
advancing, or `-1`. `write()` always returns zero and `flush()` does nothing.
Keep manually constructed payload sizes representable as `int` when using
`available()`.

The inherited [Arduino Stream interface](https://docs.arduino.cc/language-reference/en/functions/communication/stream/)
supplies helpers such as `readBytes`, `readBytesUntil`, `readString`, `find`,
`parseInt`, `parseFloat`, and timeout configuration. They use the same read
position and may wait for their configured timeout after exhausting a payload;
prefer a known `length()` for bounded binary reads. Inherited Print formatting
methods cannot send data through this read-only wrapper; send with the channel.
The board core defines the complete inherited
[Print interface](https://github.com/arduino/ArduinoCore-API/blob/master/api/Print.h),
including formatting helpers and `getWriteError()` / `clearWriteError()`.

```cpp
void receive(WsMessage& msg) {
  while (msg.available() > 0) Serial.write((uint8_t)msg.read());
}
```

<h3 id="wsmessage-metadata">Metadata and direct payload access</h3>


```cpp
int socketId() const;
bool isText() const;
bool isBinary() const;
const uint8_t* data() const;
size_t length() const;
```

`socketId()` identifies the channel slot that sent the message. `isText()` and
`isBinary()` compare the opcode to `TEXT` and `BINARY`. `data()` points to the
entire original payload; `length()` is its total byte count. Both remain
unchanged as you consume bytes with `read()`. Use explicit lengths for binary
data and text that may contain NUL; do not pass `data()` to `strlen()`.

<h2 id="sse">SseChannel</h2>


Include `<SseChannel.h>` explicitly after `<aWOT.h>`. This channel writes
server-sent events to copied, long-lived connection handles. See the
[Server Sent Events guide](/en/guide/server-sent-events.html).

<h3 id="sse-constructor">Constructor and capacities</h3>


```cpp
template <typename TClient, int N, int MAX_ROOMS = 8>
class SseChannel;
SseChannel();
```

`N` must be positive and reserves `N` client handles plus active flags and
32-bit room-membership bitmaps. `MAX_ROOMS` must be in `1..32` and defaults to
eight. The channel uses a fixed room table and does not allocate receive buffers.
`TClient` must support default construction, copying a handle to the same
connection, and the usual Client write, connected, and stop methods. Match it
to the concrete accepted client type, such as `SseChannel<WiFiClient, 4, 1>`.
The stored copy must keep the connection usable after the original handle is
destroyed. ESP32 `WiFiClient` and the bundled native `MacOSClient`, `LinuxClient`,
and `WindowsClient` adapters support this shared ownership: `add()` retains
a copy before the local accepted handle leaves scope. Explicitly stopping any
copy closes the shared connection.
Keep the channel alive for the lifetime of its connections. It has no explicit
close-all operation; arrange transport closure before discarding it. Use the
channel from a single thread, or supply your own synchronization.

<h3 id="sse-open">open</h3>


```cpp
void open(Response& res);
```

Sets status 200, `Content-Type: text/event-stream`, and `Cache-Control: no-cache`,
then calls `res.keepOpen()`. Call before emitting HTTP headers. The response is
close-delimited, without Content-Length or chunk framing. `open()` does not add
the client. After `app.process()` succeeds with `responseOpen == true`, pass
the concrete client to `add()`; stop it otherwise. Do not retain a `Response`
or the temporary client adapter made by `process(Stream*)`.

<h3 id="sse-add">add</h3>


```cpp
int add(TClient client);
```

Copies the client into the first inactive or disconnected slot, stops any
disconnected handle being replaced, clears that slot's room membership, and
invokes `onConnect(clientId)`. Returns the selected slot index, or `-1` after
stopping the supplied client when all `N` slots are connected. Stopping any
alias of a copied handle can also stop the stored connection; do not stop the
original handle after adding it.

<h3 id="sse-rooms">Rooms</h3>


```cpp
int defineRoom(const char* name);
int findRoom(const char* name) const;
const char* roomName(int roomIdx) const;
void join(int clientId, int roomIdx);
void join(int clientId, const char* name);
void leave(int clientId, int roomIdx);
void leave(int clientId, const char* name);
bool inRoom(int clientId, int roomIdx) const;
bool inRoom(int clientId, const char* name) const;
```

Rooms use the same rules as [WebSocket rooms](#ws-rooms). `defineRoom()` returns
an existing index for equal, case-sensitive name contents, even at capacity;
otherwise it assigns the next index or returns `-1` when the table is full.
Null names return `-1` without consuming capacity, and empty names are valid.
Names are borrowed and must remain alive and unchanged while the channel
exists. `findRoom()` returns an index or `-1` without defining a room.
`roomName()` returns the registered pointer or null for an unnamed or invalid
index.

Membership methods accept either a registered name or a numeric index in
`0..MAX_ROOMS-1`; numeric indices need not have names. Invalid client or room
indices and null or unknown names make `join()` and `leave()` no-ops and make
`inRoom()` return false. Membership methods do not check whether a slot is
active. Adding a client, closing it, or deactivating its slot clears membership.
One client can join several rooms. Client IDs and room indices are local to
each channel; an SSE room and a WebSocket room have independent memberships.

<h3 id="sse-send">Sending</h3>


```cpp
void send(int clientId, const char* data);
void send(int clientId, const char* event, const char* data);
void to(int roomIdx, const char* data);
void to(int roomIdx, const char* event, const char* data);
void to(const char* name, const char* data);
void to(const char* name, const char* event, const char* data);
void broadcast(const char* data);
void broadcast(const char* event, const char* data);
```

`send()` targets one active client, `to()` targets active members of a room,
and `broadcast()` targets all active clients. Invalid client or room indices,
inactive target slots, and null or unknown room names are ignored. Named
operations never create rooms; define them first.

Overloads without an event name send an unnamed event received through
`EventSource.onmessage`. Supplying an event name prefixes `event: <event>`;
browsers receive it through `addEventListener(event, ...)`. Use an event name
without CR or LF and a non-null NUL-terminated data string. A null event name
selects an unnamed event.
Each payload line is prefixed with `data: `; CR, LF, and CRLF are normalized to
LF, preserving empty and trailing lines. A final blank line ends the event.
Strings are read during the call and need not remain alive afterward. Binary
data and embedded NUL are not supported by these overloads.

All return void. Positive short writes are retried. Disconnected clients,
zero writes, or invalid write counts stop and deactivate the affected slot.
Slow transport writes can delay the loop; there is no outgoing event queue.

```cpp
events.defineRoom("updates");
events.join(0, "updates");
events.send(0, "ready");
events.to("updates", "sample", "first line\nsecond line");
events.broadcast("all clients");
```

<h3 id="sse-foreach">forEach and onConnect</h3>


```cpp
void forEach(Callback fn);
void onConnect(ConnectCb callback);
```

With `STD_FUNCTION_MIDDLEWARE`, `ConnectCb` is
`std::function<void(int clientId)>` and `Callback` is
`std::function<void(Print&)>`. Otherwise they are function pointers with those
same arguments. `onConnect()` replaces the callback; `nullptr` clears it. It
runs synchronously during `add()`, after the slot becomes active, and receives
that slot's client ID. Use it to join rooms or send an initial event to that
client. Captured objects must outlive their callbacks.

`forEach()` removes disconnected clients as it visits their slots and calls
the supplied non-null `Callback` with each remaining client's Print interface.
Do not retain that reference. It is useful for comments, custom
`id:` or `retry:` fields, and periodic cleanup. Writes through this Print
reference bypass `send()`'s short-write retries and failure detection; inspect
return counts and design custom writes accordingly.

```cpp
void initialEvent(int clientId) {
  events.join(clientId, "updates");
  events.send(clientId, "ready", "connected");
}
// During setup(): events.defineRoom("updates"); events.onConnect(initialEvent);
```

<h3 id="sse-state">Connection state and closure</h3>


```cpp
int count() const;
bool connected(int clientId) const;
void close(int clientId);
```

`count()` counts active slots and `connected()` reports whether an in-range
slot is active. Neither probes the transport. Sending methods clean up
disconnected or failed target slots, `forEach()` checks every active slot, and
`add()` can reclaim a disconnected slot. IDs are reused and are not durable
client identities. `close()` immediately stops and deactivates an active
client and clears its room membership; invalid or inactive IDs are ignored.

There is no `poll()`, disconnect callback, automatic heartbeat, event history,
or replay logic. Browsers may reconnect, but the application must implement
event IDs and replay if needed. Periodically call `forEach()` or broadcast an
application event to discover disconnected clients in otherwise idle channels.

<h2 id="storage">Storage and StreamClient</h2>


<h3 id="storage-arrayview">ArrayView</h3>


```cpp
template <typename T> class ArrayView;
ArrayView();
ArrayView(T* data, int size);
template <size_t N> ArrayView(T (&data)[N]);
T* data() const;
int size() const;
T& operator[](int index) const;
```

Defined in `<ArrayView.h>` and available through `<aWOT.h>`. The default view
has a null pointer and size zero. The pointer constructor records the pointer
and explicit size; the array constructor infers the capacity and rejects arrays
larger than `INT_MAX` at compile time. `data()` and `size()` return the recorded
values. Indexing returns a reference without bounds checks, including on a
const view. A view neither owns nor copies its storage and does not validate
pointers or sizes. Its elements must remain alive whenever accessed.

`App::ProcessOptions` uses views for URL storage, output storage, and the
contiguous `Request::Header` array. Arrays infer sizes; raw pointers require
an explicit `{pointer, size}` pair. Request headers borrow their names and
value buffers as well. Keep all of this storage, and the context, alive until
`process()` returns. Separate concurrent requests need separate capture buffers.

```cpp
char url[64];
uint8_t output[32];
char authorization[64];
Request::Header headers[] = { {"Authorization", authorization} };
App::ProcessOptions options;
options.urlBuffer = url;
options.writeBuffer = output;
options.headers = headers;
// Pointer alternatives need explicit capacities:
char* urlPointer = url;
options.urlBuffer = {urlPointer, sizeof(url)};
```

Null URL or output pointers select the default buffer; their supplied size is
ignored. A custom URL buffer needs at least two bytes; custom output capacity
must exceed `CHUNK_OVERHEAD`. Active header entries need non-null names and
buffers and positive capacities. Invalid options close the client without
running handlers or `finally`. Captured values are cleared before and after
processing. A header view with size zero ignores its pointer and entries;
negative counts and positive counts with a null array pointer are invalid.
See [ProcessOptions](#app-process-options) and
[request header capture](#req-header).

<h3 id="storage-streamclient">StreamClient</h3>


```cpp
StreamClient(Stream* stream);
int connect(IPAddress, uint16_t);
int connect(const char*, uint16_t);
size_t write(uint8_t byte);
size_t write(const uint8_t* buffer, size_t length);
int available();
int read();
int read(uint8_t* buffer, size_t length);
int peek();
void flush();
void stop();
uint8_t connected();
operator bool();
```

Defined in `<StreamClient.h>` and included by `<aWOT.h>`. This Arduino Client
adapter borrows a non-null `Stream*` that must outlive it. Single-byte reads,
peek, availability, writes, and flush delegate to that stream. The bulk read
loops until it has read `length` bytes or encounters `-1`, returning the count
read. Both `connect()` overloads return 1 without creating a network
connection; `connected()` always returns 1 and boolean conversion returns
true. `stop()` does nothing. The adapter cannot detect stream disconnection
or close a serial port.

`App::process(Stream*)` creates a temporary adapter; `req.client()` points to
it only during processing. Never retain that pointer or put it in a channel.
For a persistent adapter, create one in persistent storage and pass its Client
pointer explicitly, keeping its wrapped stream alive. Its always-connected
semantics still require application-managed termination.

The inherited [Arduino Stream methods](https://docs.arduino.cc/language-reference/en/functions/communication/stream/)
and [Print formatting and write-error methods](https://github.com/arduino/ArduinoCore-API/blob/master/api/Print.h)
are available according to your board core. The adapter's delegated writes do
not translate the wrapped stream's write-error flag into its own Print flag.

<h2 id="configuration">Configuration</h2>


The [configuration guide](/en/guide/configuration.html) shows a complete Uno
sketch and build-wide overrides. Prefer per-request storage when choosing
different URL/output capacities for individual requests.

<h3 id="configuration-namespace">Headers and namespace</h3>


All library types are defined in `namespace awot`. `<aWOT.h>` includes App,
Router, Request, Response, StreamClient, and common configuration. It adds
`using namespace awot;` unless `ENABLE_AWOT_NAMESPACE` is defined. That macro
controls the convenience using-directive; it does not change where types are
declared. With it defined, write `awot::App`, `awot::Request`, and so on, or use
your own selective using-declarations. ArrayView arrives through Request.
Include `<WsChannel.h>`, `<WsMessage.h>`, or `<SseChannel.h>` explicitly for
those APIs.

<h3 id="configuration-callbacks">Callback selection</h3>


`aWOT_common.h` selects `STD_FUNCTION_MIDDLEWARE` automatically on ESP8266,
ESP32, or compilers whose `__has_include` finds `<functional>`, unless
`AWOT_NO_STD_FUNCTION_MIDDLEWARE` is defined. Explicitly selecting
`STD_FUNCTION_MIDDLEWARE` also requires `<functional>`. In that mode, router
middleware and channel callbacks use `std::function`: free functions, lambdas
with captures, and supported member bindings are accepted. Captured references
and bound instances must outlive their registrations. `App::ErrorHandler`
always remains a function pointer.

Without std::function, middleware and channel callbacks are function pointers.
Use free/static functions or convertible lambdas without captures. The member
binding overloads are unavailable. To force this configuration, pass
`-DAWOT_NO_STD_FUNCTION_MIDDLEWARE` to **every library and sketch C++ translation
unit**. A define at the top of an `.ino` cannot change the compiled library ABI.
Defining both selection macros is a compile-time error.

<h3 id="configuration-http">HTTP storage macros</h3>


| Macro | Low-memory default | Other default | Meaning |
| --- | --- | --- | --- |
| `SERVER_URL_BUFFER_SIZE` | 64 | 256 | Default combined URL storage, including NUL. |
| `SERVER_PUSHBACK_BUFFER_SIZE` | 32 | 128 | Bytes reserved in every Request's pushback stack. |
| `SERVER_OUTPUT_BUFFER_SIZE` | 32 | 1024 | Default response output buffer, including chunk overhead. |
| `SERVER_MAX_HEADERS` | 10 | 10 | Caller-supplied response header slots. |

`LOW_MEMORY_MCU` is selected for the ATmega328P, ATmega32U4, ATmega16U4,
and the `_AVR_ATmega328__` target macro listed in `aWOT_common.h`. Those defaults
apply to Uno and Leonardo; they are not measurements of total sketch memory.
Default content type, connection, and framing headers do not use caller header
slots. `Response::set()` silently ignores additional headers once
`SERVER_MAX_HEADERS` is reached. Request header capture capacity is supplied
separately by the caller.

HTTP overrides affect object layout and/or implementation. Apply them
consistently to **every library and sketch translation unit** and rebuild the
library. Use positive pushback/header capacities and a default URL capacity
of at least two bytes. Output storage must exceed `CHUNK_OVERHEAD`.

`CHUNK_HEADER_SIZE` is derived from `SERVER_OUTPUT_BUFFER_SIZE`: it is 3 for
capacities through `0xF`, 4 through `0xFF`, 5 through `0xFFF`, 6 through
`0xFFFF`, and 10 above that. `CHUNK_TRAILER_SIZE` is 2;
`CHUNK_OVERHEAD` is their sum; `CHUNK_DATA_SIZE` subtracts the overhead from
the default output capacity. These are derived macros, not separate override
points. The chunk-header reservation also limits custom output buffers.
`CRLF` is the `"\r\n"` protocol string.

<h3 id="configuration-flash">P and flash strings</h3>


```cpp
P(message) = "Hello from flash!";
// Inside a route handler:
res.printP(message);
```

`P(name)` declares a static `const unsigned char` array in program memory. On
AVR it uses a named `.progmem` section; elsewhere it uses `PROGMEM`. It is a
declaration macro; supply the initializer and semicolon. Use `printP()` for a
NUL-terminated string and `writeP(data, length)` for exact bytes. Flash storage
does not change the pointer lifetime requirements for request or response
headers; their ordinary C strings must be readable in the platform's normal
address space.

<h3 id="configuration-channels">Channel macros</h3>


| Macro | Default | Meaning |
| --- | --- | --- |
| `WS_RX_BUFFER_SIZE` | 256 | Complete incoming message capacity in bytes, including all fragments; allocated in every WebSocket slot. |
| `WS_PING_INTERVAL_MS` | `5000UL` | Time from the previous heartbeat ping to sending the next when no pong is pending. |
| `WS_PING_TIMEOUT_MS` | `10000UL` | Time to wait for the outstanding heartbeat pong before stopping the connection. |

Define these before including `<WsChannel.h>`. Use positive capacities and
intervals appropriate for your application. Setting a heartbeat interval to
zero does not disable heartbeat; it makes a ping due immediately. Channel
definitions must agree across translation units that instantiate the same
channel type. Both `WsChannel<TClient, N, MAX_ROOMS>` and
`SseChannel<TClient, N, MAX_ROOMS>` use template parameters for slot and room
capacities; these are not macros. SSE has no automatic heartbeat configuration.
Each WebSocket slot also reserves a separate 125-byte control buffer. Large
capacities can exhaust small boards; the Uno SerialBridge example uses one
slot with a 64-byte message buffer plus the control buffer.
