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

```arduino
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
The parser handles `Content-Length` internally rather than populating a capture
for it; use `req.left()` to inspect the remaining body length.

<span id="req-variables"></span>
<h3 id="req-context">Request::context</h3>

```cpp
void* context;
```

The pointer supplied by `ProcessOptions.context`, defaulting to `nullptr`.
aWOT neither owns nor copies the referenced object. Keep it alive through
processing, use the correct type when casting, and check for null when the
context is optional.

```arduino
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
the Request's body bounds, pushback, and counters. Use `req` for an ordinary
body; direct Client I/O is useful after a protocol upgrade or when continuing a
response opened with `res.keepOpen()`.

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

```arduino
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

```arduino
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

```arduino
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

```arduino
void saveForm(Request& req, Response& res) {
  char name[16];
  char value[32];
  while (req.left() > 0) {
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

Return the number of unread body bytes declared by Content-Length plus pushed
back bytes. With no Content-Length, the ordinary body length is zero. aWOT
accepts nonnegative decimal Content-Length up to `INT_MAX`, with surrounding
spaces/tabs. Repeated values must agree. Comma lists, conflicting values, and
every incoming Transfer-Encoding produce 400 and close the connection;
chunked request bodies are unsupported.

Reading less than the declared length prevents keep-alive reuse. aWOT does not
automatically drain an unread or truncated body.

<h3 id="req-available">Request::available</h3>

```cpp
int available();
```

Return immediately available body bytes, capped at the remaining Content-Length,
plus pushback bytes. Zero can mean the body is complete or that its next bytes
have not arrived; use `left()` to distinguish them.

<h3 id="req-read">Request::read</h3>

```cpp
int read();
int read(uint8_t* buf, size_t size);
```

The byte overload reads pushback first, then the Client. Return 0–255 for a
byte or -1 when no byte is available or the body is exhausted. It does not wait
for more input. Reaching the body's end through this overload sets the inherited
Stream timeout to zero.

The buffer overload returns the number read, zero at body end, or the underlying
Client's -1 when no buffered byte could be returned. It can read fewer bytes than
requested. Both overloads stop at the body boundary and leave a following HTTP
request in the transport. Supply valid storage for the requested buffer size.
For a timed bounded read, use [`readBytes()`](#req-readBytes) and check its count.

<h3 id="req-peek">Request::peek</h3>

```cpp
int peek();
```

Read one byte and push it back, returning that byte or -1. The next Request read
returns it again. A first peek can increase `bytesRead()` because it initially
reads the underlying Client, while `left()` remains unchanged.

<h3 id="req-push">Request::push</h3>

```cpp
void push(uint8_t ch);
```

Push a byte onto the Request's LIFO pushback stack. Reading it again does not
increase `bytesRead()`. `left()` and `available()` include pushed bytes. The
capacity is `SERVER_PUSHBACK_BUFFER_SIZE`; when full, a push replaces the
previous top byte with the new one and has no failure return.

<h3 id="req-bytesRead">Request::bytesRead</h3>

```cpp
int bytesRead();
```

Return bytes read from the underlying Client by this Request, including the
request line, headers, and body. Pushback rereads are not counted again.

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
