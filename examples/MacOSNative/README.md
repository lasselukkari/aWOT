# macOS Native aWOT Example

This example demonstrates running the aWOT web server library on macOS without Arduino hardware, using native BSD sockets and multithreading.

## Prerequisites

- macOS with Xcode Command Line Tools (`xcode-select --install`)
- clang++ (comes with Xcode CLT)

## Setup

The ArduinoCore-API sources are included in this example. Its String files use
`WString.h` / `WString.cpp` to avoid the macOS case-insensitive `string.h`
collision. No additional download or renaming is needed.

## Build & Run

```bash
make        # Build the example
make run    # Run the server
make test   # Run socket/shim regressions and an HTTP smoke test
make clean  # Clean build artifacts
```

## Test

`make test` exercises socket ownership, disconnect handling, request timeouts,
address conversion, and numeric formatting before starting its own HTTP server.
Port 8080 must be free; startup fails if another process already owns it.

Once running, visit http://localhost:8080/ in your browser or:

```bash
curl http://127.0.0.1:8080/
```

Expected output:
```html
<html>
<head>
<title>Hello World!</title>
</head>
<body>
<h1>Greetings middle earth!</h1>
</body>
</html>
```

## Benchmarking

```bash
# Install wrk (modern HTTP benchmarking tool)
brew install wrk

# Run benchmark
wrk -t4 -c10 -d10s http://localhost:8080/
```

## Files

| File | Description |
|------|-------------|
| `Arduino.h` | Compatibility shim providing `millis()`, `delay()`, `itoa()`, etc. |
| `MacOSSocket.h/cpp` | BSD socket implementation of Arduino Client/Server API |
| `HelloWorld.cpp` | The example application |
| `Makefile` | Build system |
| `tests/` | Native socket and compatibility regression tests |
| `ArduinoCore-API/` | Arduino Core API (external dependency) |

## How It Works

The example uses:

1. **Arduino.h** - A thin shim that provides macOS implementations of Arduino functions (`millis()`, `delay()`, `yield()`, `itoa()` family, `dtostrf()`)

2. **ArduinoCore-API** - The official Arduino API classes (`Print`, `Stream`, `Client`, `IPAddress`, `String`)

3. **MacOSClient/MacOSServer** - BSD socket wrappers that implement the Arduino `Client` interface, allowing aWOT to work unchanged

4. **Multithreading** - Each client connection is handled in a separate thread using `std::thread`

The main thread waits for connections without polling continuously. Socket reads
return immediately when no data is ready, allowing aWOT's request timeouts to
expire. Client and server objects close their sockets when destroyed.

Each native client uses a 1 KiB receive buffer to avoid a socket call for every
parsed byte. Buffered data stays available through both the request and raw
client read APIs, including after the peer disconnects.

## Windows Portability

The socket implementation uses POSIX/BSD socket APIs. For Windows:

- Replace `<sys/socket.h>`, `<netinet/in.h>`, etc. with `<winsock2.h>`
- Replace `close()` with `closesocket()`
- Add `WSAStartup()`/`WSACleanup()` initialization
- Handle `EAGAIN`/`EWOULDBLOCK` as `WSAEWOULDBLOCK`

## License

Same as the aWOT library (MIT).
