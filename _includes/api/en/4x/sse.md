<h2 id="sse">SseChannel</h2>

Include `<SseChannel.h>` explicitly after `<aWOT.h>`. This channel writes
server-sent events to copied, long-lived connection handles. See the
[complete SSE guide](/en/guide/server-sent-events.html).

<h3 id="sse-constructor">Constructor and capacity</h3>

```arduino
template <typename TClient, int N>
class SseChannel;
SseChannel();
```

`N` must be positive and reserves `N` client handles plus active flags.
`TClient` must support default construction, copying a handle to the same
connection, and the usual Client write, connected, and stop methods. Match it
to the concrete accepted client type, such as `SseChannel<WiFiClient, 4>`.
Keep the channel alive for the lifetime of its connections. It has no explicit
close-all operation; arrange transport closure before discarding it. Use the
channel from a single thread, or supply your own synchronization.

<h3 id="sse-open">open</h3>

```arduino
void open(Response& res);
```

Sets status 200, `Content-Type: text/event-stream`, and `Cache-Control: no-cache`,
then calls `res.keepOpen()`. Call before emitting HTTP headers. The response is
close-delimited, without Content-Length or chunk framing. `open()` does not add
the client. After `app.process()` succeeds with `responseOpen == true`, pass
the concrete client to `add()`; stop it otherwise. Do not retain a `Response`
or the temporary client adapter made by `process(Stream*)`.

<h3 id="sse-add">add</h3>

```arduino
void add(TClient client);
```

Copies the client into the first inactive or disconnected slot, stops any
disconnected handle being replaced, and invokes `onConnect`. When all `N`
slots are connected, it stops the supplied client. It returns no success
flag. Stopping any alias of a copied handle can also stop the stored connection;
do not stop the original handle after adding it.

<h3 id="sse-send">send</h3>

```arduino
void send(const char* data);
void send(const char* event, const char* data);
```

The first overload broadcasts an unnamed event; browsers receive it through
`onmessage`. The second prefixes `event: <event>` for a named event. Use a
non-null event name without CR or LF, and a non-null NUL-terminated data string.
Each payload line is prefixed with `data: `; CR, LF, and CRLF are normalized to
LF, preserving empty and trailing lines. A final blank line ends the event.
Strings are read during the call and need not remain alive afterward. Binary
data and embedded NUL are not supported by these overloads.

Both return void. Positive short writes are retried. Disconnected clients,
zero writes, or invalid write counts stop and deactivate the affected slot.
Slow transport writes can delay the loop; there is no outgoing event queue.

```arduino
events.send("ready");
events.send("sample", "first line\nsecond line");
```

<h3 id="sse-foreach">forEach and onConnect</h3>

```arduino
void forEach(Callback fn);
void onConnect(Callback callback);
```

The public type is `typedef std::function<void(Print&)> Callback` when
`STD_FUNCTION_MIDDLEWARE` is selected, or `typedef void (*Callback)(Print&)`
otherwise. `onConnect()` replaces the callback; `nullptr` clears it. The
callback receives a reference to the stored client's Print interface, runs
synchronously during `add()`, and can write an initial event or comment.
Captured objects must outlive their callbacks; do not retain the Print reference.

`forEach()` first removes disconnected clients, then calls the supplied
non-null function for each active client. It is useful for comments, custom
`id:` or `retry:` fields, and periodic cleanup. Writes through this Print
reference bypass `send()`'s short-write retries and failure detection; inspect
return counts and design custom writes accordingly.

```arduino
void initialEvent(Print& client) {
  client.print("event: ready\ndata: connected\n\n");
}
// Register during setup(): events.onConnect(initialEvent);
```

There is no `poll()`, disconnect callback, automatic heartbeat, event history,
or replay logic. `send()` and `forEach()` remove disconnected clients; `add()`
can reuse them. Browsers may reconnect, but the application must implement
event IDs and replay if needed.
