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

```arduino
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
| `WS_RX_BUFFER_SIZE` | 256 | Receive payload capacity in bytes, allocated in every WebSocket slot. |
| `WS_PING_INTERVAL_MS` | `5000UL` | Time from the previous heartbeat ping to sending the next when no pong is pending. |
| `WS_PING_TIMEOUT_MS` | `10000UL` | Time to wait for the outstanding heartbeat pong before stopping the connection. |

Define these before including `<WsChannel.h>`. Use positive capacities and
intervals appropriate for your application. Setting a heartbeat interval to
zero does not disable heartbeat; it makes a ping due immediately. Channel
definitions must agree across translation units that instantiate the same
channel type. `N` and `MAX_ROOMS` are template parameters, not macros;
SseChannel only has its client type and capacity template parameters and no
heartbeat configuration. Large WebSocket capacities can exhaust small boards;
the Uno SerialBridge example uses one slot and 64 receive bytes.
