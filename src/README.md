# aWOT 4.0 contracts

## Request and buffer contracts

- `ProcessOptions` buffer and header fields are non-owning views. Assigning an
  array infers its capacity or entry count; `{pointer, size}` supplies it
  explicitly. A null buffer pointer selects default storage with its own
  capacity; its unused size is ignored. A supplied URL buffer needs at least
  two bytes, and a supplied output buffer needs more than `CHUNK_OVERHEAD` bytes.
  Invalid options close the client without invoking handlers. Caller buffers
  avoid the default arrays; convenience helpers allocate only missing buffers.
- `Request::Header` captures a named header in caller-owned storage. Its
  `{name, array}` constructor infers the buffer capacity; `{name, pointer, size}`
  supplies it explicitly. `headers` is a contiguous array with an inferred or
  explicit entry count. Active entries require non-null names/buffers and
  positive capacities. Their buffers and presence state are cleared before
  parsing and after processing. A zero count ignores the pointer and all entries.
  The array, names, and buffers must remain alive throughout `process()`, and
  concurrent invocations require separate storage.
- `get(name)` returns `nullptr` for an absent or uncaptured header. A captured
  header present with an empty value returns a non-null pointer to an empty
  string. Header values are available only while the request is being processed.
- `path()` returns the percent-decoded path. The literal `?` boundary is found
  before decoding. `query()` returns the raw encoded query without `?`.
  `query(name, buffer, length)` matches decoded names and returns decoded values;
  it treats `+` as space, while `path()` preserves literal `+`. Invalid percent
  escapes and decoded NUL anywhere in a request target produce HTTP 400.
- Form/query capacities include the NUL terminator. Invalid pointers or
  nonpositive capacities fail without touching buffers. Overflow returns false
  with a terminated partial result. Form overflow consumes through the field's
  raw `&` delimiter or end of body so a subsequent call can read the next field.
- Content-Length accepts decimal digits with optional surrounding spaces/tabs,
  bounded by `INT_MAX`. Repeated fields must agree. Comma lists, conflicting
  lengths, and every request Transfer-Encoding are rejected with HTTP 400 and
  connection closure. Chunked request decoding remains unsupported.
- The advanced `SERVER_PUSHBACK_BUFFER_SIZE` override must be at least 18 bytes
  and must be applied to every library and sketch translation unit. Defaults are
  32 bytes on low-memory MCUs and 128 bytes elsewhere.
- Routers and Apps cannot be copied. Mount with `app.use(path, &router)` or
  `router.use(path, &child)`; the formerly ignored `Router(path)` constructor
  has been removed.

## Responses and channels

Output capacity is bounded by both caller storage and the reserved hexadecimal
chunk header. Large writes are split into representable chunks. Default content
type and connection/framing headers do not consume caller header slots. The
generated WebSocket accept header does not consume a slot either.

Positive short transport writes are retried from the accepted offset. A zero
write stops immediately, sets the inherited Print `getWriteError()`, and closes
the client. Failed responses never return `keepAlive` or `responseOpen`.
Buffered writes report bytes accepted into the buffer; errors during later
`flush()` or finalization close the connection. `bytesSent()` remains an accepted
byte counter (including HTTP status/headers), saturating at `INT_MAX`.

`status(100)`, `status(102)`, and `status(103)` emit an interim response and reset
state for a final response. Headers queued before that call belong only to the
interim response and are cleared afterward. Status 101 retains its upgrade state
and never emits a terminal HTTP chunk.

HEAD responses and statuses that forbid a body emit neither payload bytes nor
chunk terminators. Calling `keepOpen()` for a HEAD request is ignored, and
`ProcessResult.responseOpen` remains false. Calling `setDefaults()` before a
status still chooses framing for the eventual response. An explicit
`Transfer-Encoding: chunked` response header uses the same chunk framing as an
automatically selected encoding.

`keepOpen()` streams use close-delimited HTTP bodies: Connection is set to close,
and Content-Length/Transfer-Encoding are removed before headers are emitted.
Open a stream before sending headers; use `ProcessResult.responseOpen` to retain
the client until the stream finishes. WebSocket upgrades keep their 101 response
and Upgrade connection header.

WebSockets keep one outstanding heartbeat ping until a pong or timeout, reject
control-frame extended lengths, and require an entire `upgrade` Connection token.

Both WebSocket and SSE channels use `send(clientId, ...)` for one client,
`to(room, ...)` for room members, and `broadcast(...)` for every active client.
SSE's former broadcast `send(data)` and `send(event, data)` overloads have been
replaced by `broadcast(data)` and `broadcast(event, data)`. Named SSE events also
have directed `send(clientId, event, data)` and `to(room, event, data)` overloads.

To use a room by name, register it with `defineRoom(name)`. It returns
the existing index for a duplicate name, or `-1` for a null name or when no room
slots remain. Names are case-sensitive, borrowed strings that must remain valid
and unchanged for the channel's lifetime. `findRoom(name)` returns the registered
index, or `-1` for a null or unknown name. `to()`, `join()`, `leave()`, and `inRoom()`
accept either a room index or a name. Null or unknown names send nothing and leave
memberships unchanged; `inRoom()` returns false. Name lookups use the existing
fixed room table without allocating memory. Each channel owns its own registry
and memberships through the same internal helper. Memberships are cleared when
a client slot is disconnected, closed, or reused. Numeric membership operations
accept any room index in `0..MAX_ROOMS-1`, including unnamed rooms. Both channel
templates have a `MAX_ROOMS` parameter, defaulting to 8, limited to `1..32`.

SSE prefixes each payload line with `data:`, normalizing CR, LF, and CRLF without
allocating a payload copy, including empty and trailing lines.
`SseChannel::add(client)` returns the assigned client ID or `-1` after stopping a
client rejected because capacity is full. SSE's `onConnect` callback now receives
that ID, matching WebSockets; it can send an initial event or join rooms.
`forEach` retains its `Print&` callback for raw SSE fields and comments. Raw writes
bypass the channel's short-write handling and failure detection.

Both channels expose `count()`, `connected(clientId)`, and `close(clientId)`.
Counts and connection queries report active slots until a channel operation
observes transport disconnection. SSE `close()` stops the transport immediately;
WebSocket `close()` writes a protocol close frame first. SSE has no `poll()` or
disconnect callback; sending, `forEach()`, and `add()` detect disconnected clients.

## Callback configuration and validation

Automatic `std::function` selection remains the default on supported platforms.
Advanced builds can select function pointers with
`-DAWOT_NO_STD_FUNCTION_MIDDLEWARE`, applied to **every** library and sketch C++
translation unit. Do not define this only inside a sketch. Defining both it and
`STD_FUNCTION_MIDDLEWARE` is an error.

Native tests live in `.extras/test/`; their build commands and configurations
are listed in the [unit-test workflow](../.github/workflows/NativeUnitTests.yml).
Arduino examples are compiled separately for the boards listed in the
[examples workflow](../.github/workflows/CompileExamples.yml).
