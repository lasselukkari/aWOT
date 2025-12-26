# macOS Native aWOT Example

This example demonstrates running the aWOT web server library on macOS without Arduino hardware, using native BSD sockets and multithreading.

## Prerequisites

- macOS with Xcode Command Line Tools (`xcode-select --install`)
- clang++ (comes with Xcode CLT)

## Setup

This example requires the Arduino Core API. Download and extract it:

```bash
# Download ArduinoCore-API and place in this directory as ArduinoCore-API/
# Then rename String.h -> WString.h to avoid macOS case-insensitive conflict:
cd ArduinoCore-API/api
mv String.h WString.h
mv String.cpp WString.cpp
# Update includes in the API files:
for f in *.h *.cpp; do sed -i '' 's/#include "String.h"/#include "WString.h"/g' "$f" 2>/dev/null; done
```

## Build & Run

```bash
make        # Build the example
make run    # Run the server
make test   # Run with automated test
make clean  # Clean build artifacts
```

## Test

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
| `ArduinoCore-API/` | Arduino Core API (external dependency) |

## How It Works

The example uses:

1. **Arduino.h** - A thin shim that provides macOS implementations of Arduino functions (`millis()`, `delay()`, `yield()`, `itoa()` family, `dtostrf()`)

2. **ArduinoCore-API** - The official Arduino API classes (`Print`, `Stream`, `Client`, `IPAddress`, `String`)

3. **MacOSClient/MacOSServer** - BSD socket wrappers that implement the Arduino `Client` interface, allowing aWOT to work unchanged

4. **Multithreading** - Each client connection is handled in a separate thread using `std::thread`

## Windows Portability

The socket implementation uses POSIX/BSD socket APIs. For Windows:

- Replace `<sys/socket.h>`, `<netinet/in.h>`, etc. with `<winsock2.h>`
- Replace `close()` with `closesocket()`
- Add `WSAStartup()`/`WSACleanup()` initialization
- Handle `EAGAIN`/`EWOULDBLOCK` as `WSAEWOULDBLOCK`

## License

Same as the aWOT library (MIT).
