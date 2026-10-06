<h2 id="app">App</h2>

`App` registers routes and synchronously processes one HTTP request at a time.
Include `<aWOT.h>`. The examples below use its default namespace imports; when
`ENABLE_AWOT_NAMESPACE` is defined, qualify types with `awot::`.

```arduino
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

```arduino
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

```arduino
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
underlying client for the stream or protocol upgrade.

`keepAlive` means parsing succeeded, the request body was completely consumed,
writes succeeded, the response remains ordinary HTTP, and the response selected
`Connection: keep-alive`. HTTP/1.1 alone does not enable reuse. The two flags
cannot both be true. A false flag pair means the caller should close the client.
These flags describe processing, rather than guaranteeing that a peer remains
connected; the caller owns later disconnect handling and idle timeouts.

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

```arduino
// Once client is connected:
App::ProcessResult result = app.process(&client);
if (!result.responseOpen && !result.keepAlive) {
  client.stop();
}
// For keepAlive, retain this client and process its next request when ready.
// For responseOpen, let the stream/channel own subsequent I/O and closure.
```

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
