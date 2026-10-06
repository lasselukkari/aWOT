<h2 id="ws">WsChannel</h2>

Include `<WsChannel.h>` explicitly, after `<aWOT.h>`. A channel holds a fixed
number of copied transport handles and receives frames when you call `poll()`.
See the [complete WebSocket guide](/en/guide/websockets.html).

<h3 id="ws-constructor">Constructor and capacities</h3>

```arduino
template <typename TClient, int N, int MAX_ROOMS = 8>
class WsChannel;
WsChannel();
```

`TClient` must be the exact concrete client type supplied to `App::process()`;
`upgrade()` casts `req.client()` to that type. It must support default
construction and copying a handle to the same underlying connection. For
example, use `WsChannel<WiFiClient, 4, 1>` for requests delivered as `WiFiClient`.
Do not pass the temporary adapter from `process(Stream*)` to such a channel.
Keep the channel alive while it owns connections; close active sockets before
discarding it. A channel is intended for single-threaded use.

`N` must be positive. `MAX_ROOMS` must be in `1..32`; each socket has a 32-bit
room-membership bitmap. Each of the `N` slots allocates a
`WS_RX_BUFFER_SIZE`-byte receive buffer, even when idle. The default receive
capacity is 256 bytes per slot. See [configuration](#configuration-channels).

<h3 id="ws-upgrade">upgrade</h3>

```arduino
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

```arduino
void websocketRoute(Request& req, Response& res) {
  if (!sockets.upgrade(req, res)) res.sendStatus(400);
}
```

<h3 id="ws-poll">poll and frame handling</h3>

```arduino
void poll();
```

Call frequently from `loop()`. It reads available bytes incrementally without
waiting for a whole frame, dispatches completed text/binary messages, replies to
ping and close frames, and checks disconnections and heartbeat timeouts.
Positive short writes are retried; a zero or invalid write deactivates the slot,
stops its client, and calls `onDisconnect`.

Incoming frames must be masked and unfragmented (`FIN = 1`); continuation frames
are unsupported. Protocol errors close with 1002; fragmented data closes with
1003; payloads larger than `WS_RX_BUFFER_SIZE` close with 1009. Text and close
reasons must be valid UTF-8 or the channel closes with 1007. Control payloads
must fit both the 125-byte protocol limit and the receive capacity. Compression
extensions and TLS are not supplied by the channel; the example uses plain
WebSockets. Outgoing payloads are unmasked and are not limited by the receive
buffer size. Keep outgoing lengths within the 32-bit frame-length implementation.

The default heartbeat sends an empty ping after 5000 ms and waits for a pong
for up to 10000 ms. There is at most one outstanding heartbeat ping. Timeout
stops the connection without sending a close frame. Long gaps between `poll()`
calls delay all frame processing and heartbeat checks.

<h3 id="ws-rooms">Rooms</h3>

```arduino
int defineRoom(const char* name);
const char* roomName(int roomIdx) const;
void join(int socketId, int roomIdx);
void leave(int socketId, int roomIdx);
bool inRoom(int socketId, int roomIdx) const;
```

`defineRoom()` returns the next index, starting at zero, or `-1` when
`MAX_ROOMS` definitions have been used. It borrows `name`; use a string literal
or storage that outlives the channel. It does not deduplicate names.
`roomName()` returns that pointer, or null for an undefined or out-of-range
index. Room membership operations accept any room index in `0..MAX_ROOMS-1`,
even before it is named. Invalid socket or room indices make `join()` and
`leave()` no-ops and make `inRoom()` return false. These membership methods do
not check whether a socket is active; new upgrades and disconnections clear
membership. Room indices and socket slot IDs are local to this channel.

```arduino
int chatRoom = sockets.defineRoom("chat");
// Check chatRoom >= 0 before using it.
```

<h3 id="ws-send">Sending</h3>

```arduino
void send(int socketId, const char* text);
void send(int socketId, const uint8_t* data, size_t n, bool binary = false);
void to(int roomIdx, const char* text);
void to(int roomIdx, const uint8_t* data, size_t n, bool binary = false);
void broadcast(const char* text);
void broadcast(const uint8_t* data, size_t n, bool binary = false);
```

`send()` targets one active socket, `to()` targets active members of a room,
and `broadcast()` targets every active socket. They return no delivery result;
failed writes deactivate the affected connection and call `onDisconnect`.
Invalid indices and inactive `send()` targets are ignored. String overloads
require a non-null NUL-terminated string and send a text frame. Buffer overloads
send exactly `n` bytes; pass `true` for binary. Supply valid storage until the
method returns. Outgoing text must be valid UTF-8; these methods do not validate
it. A received payload can contain NUL bytes, so use its pointer
and length rather than the string overload.

```arduino
void echo(WsMessage& msg) {
  sockets.send(msg.socketId(), msg.data(), msg.length(), msg.isBinary());
}
```

<h3 id="ws-state">Connection state and closure</h3>

```arduino
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

```arduino
void onConnect(ConnectCb cb);
void onDisconnect(DisconnectCb cb);
void onMessage(MessageCb cb);
```

With automatic `std::function` support, the public callback types are:

```arduino
typedef std::function<void(int socketId)> ConnectCb;
typedef std::function<void(int socketId)> DisconnectCb;
typedef std::function<void(WsMessage& msg)> MessageCb;
```

With function-pointer callbacks they are:

```arduino
typedef void (*ConnectCb)(int socketId);
typedef void (*DisconnectCb)(int socketId);
typedef void (*MessageCb)(WsMessage& msg);
```

Each setter replaces the previous callback; `nullptr` clears it. `onConnect`
runs during a successful upgrade, after the slot becomes active.
`onDisconnect` runs after deactivation and transport closure.
`onMessage` runs synchronously inside `poll()` for completed text and binary
messages. The [WsMessage](#wsmessage) object and its data must not be retained
after that callback. Captured objects in callbacks must outlive their use.
