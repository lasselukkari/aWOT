<h2 id="wsmessage">WsMessage</h2>

`WsMessage` is a read-only Arduino `Stream` over a completed WebSocket payload.
`<WsChannel.h>` includes it, or include `<WsMessage.h>` explicitly. A channel
creates a temporary message for `onMessage`; copy bytes during the callback if
you need them later. The payload is not NUL-terminated.

<h3 id="wsmessage-constructor">Constructor and opcodes</h3>

```arduino
enum Opcode { TEXT = 0x1, BINARY = 0x2 };
WsMessage(uint8_t* buffer, size_t length, uint8_t opcode, int socketId);
```

The constructor borrows `buffer`, starts the read position at zero, and records
the supplied metadata. It does not validate the pointer or opcode. The buffer
must contain at least `length` readable bytes and outlive this message. Channel
callbacks receive only validated text or binary frames.

<h3 id="wsmessage-read">Reading and Stream methods</h3>

```arduino
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

```arduino
void receive(WsMessage& msg) {
  while (msg.available() > 0) Serial.write((uint8_t)msg.read());
}
```

<h3 id="wsmessage-metadata">Metadata and direct payload access</h3>

```arduino
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
