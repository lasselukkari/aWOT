<h2 id="storage">Storage and StreamClient</h2>

<h3 id="storage-arrayview">ArrayView</h3>

```arduino
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

```arduino
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

```arduino
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
