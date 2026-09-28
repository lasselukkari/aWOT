# aWOT snapshot notes

The fixes in [REVIEW.md](REVIEW.md) retain the existing payload buffers and
callback defaults. FridgeCure uses this snapshot for its local HTTP API through `ApiServer`.

## Request and buffer contracts

- A null `ProcessOptions` buffer pointer selects default storage with its own
  capacity; its unused length is ignored. A supplied URL buffer needs at least
  two bytes, and a supplied output buffer needs more than `CHUNK_OVERHEAD` bytes.
  Invalid options close the client without invoking handlers. Caller buffers
  avoid the default arrays; convenience helpers allocate only missing buffers.
- `headers` is a contiguous array with `headerCount` active entries. Active
  entries require non-null names/buffers and positive capacities. Their buffers
  are cleared before parsing and after processing. A zero count ignores the
  pointer and all entries, including `next`.
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
- Routers and Apps cannot be copied. Mount with `app.use(path, &router)` or
  `router.use(path, &child)`; the formerly ignored `Router(path)` constructor
  has been removed.

## Responses and channels

Output capacity is bounded by both caller storage and the reserved hexadecimal
chunk header. Large writes are split into representable chunks. Default content
type and connection/framing headers do not consume caller header slots.

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

WebSockets keep one outstanding heartbeat ping until a pong or timeout, reject
control-frame extended lengths, and require an entire `upgrade` Connection token.
SSE prefixes each payload line with `data:`, normalizing CR, LF, and CRLF without
allocating a payload copy, including empty and trailing lines.

## Callback configuration and validation

Automatic `std::function` selection remains the default on supported platforms.
Advanced builds can select function pointers with
`-DAWOT_NO_STD_FUNCTION_MIDDLEWARE`, applied to **every** library and sketch C++
translation unit. Do not define this only inside a sketch. Defining both it and
`STD_FUNCTION_MIDDLEWARE` is an error.

Run from the repository root:

```sh
npx vitest run firmware/test/awot.test.mjs
```

The suite compiles the library as C++11 against host Arduino/Client stubs and
runs with AddressSanitizer and UndefinedBehaviorSanitizer. It covers default
callbacks, function pointers, and a 32-byte default output/pushback configuration.
It checks small and large custom buffers, invalid options, raw/decoded URL
boundaries, captured headers, body framing, router ownership, short/failed writes,
interim/final responses, WebSocket frames/heartbeat clock rollover, and SSE output.

The stubs model deterministic I/O and a simulated clock, not network timing or
hardware. Arduino binary size, stack usage, and on-device behavior have not been
measured. Serial diagnostics remain absent.
