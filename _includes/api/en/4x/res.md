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

```arduino
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

```arduino
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
Use the [SSE](/en/guide/server-sent-events.html) or
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

```arduino
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

```arduino
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
