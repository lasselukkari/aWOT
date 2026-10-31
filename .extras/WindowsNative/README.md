# Windows Native aWOT Example

Run the macOS Hello World example on Windows without Arduino hardware. The
Windows adapter uses Winsock, implements Arduino's `Client` interface, and
handles each connection in a separate `std::thread`. The aWOT library sources
run unchanged.

## Prerequisites

Install [MSYS2](https://www.msys2.org/) and open its **UCRT64** terminal. Install
the native Windows build tools and Python for the HTTP test:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-python
```

Use the UCRT64/MinGW toolchain rather than the MSYS POSIX compiler. This example
supports MinGW GCC; the bundled ArduinoCore API uses GNU attributes and has not
been adapted for MSVC. See [MSYS2's CMake instructions](https://www.msys2.org/docs/cmake/).

The build uses the ArduinoCore-API sources bundled in
[`../ArduinoCore-API`](../ArduinoCore-API/README.md), shared by all native
examples. No separate Arduino installation or download is needed.

For the other desktop platforms, see the [macOS native example](../MacOSNative/README.md)
or [Linux native example](../LinuxNative/README.md).

## Build and run

From the repository root in the UCRT64 terminal:

```bash
cmake -S .extras/WindowsNative -B .extras/WindowsNative/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build .extras/WindowsNative/build --parallel 4
./.extras/WindowsNative/build/HelloWorld.exe
```

Visit <http://127.0.0.1:8080/>. The page displays **Greetings middle earth!**.
Stop the server with Ctrl+C. The executable also runs from PowerShell or Explorer;
its GCC, C++ and thread runtimes are linked statically.

The server always listens on port 8080 and exits with an error if that port is
occupied. Stop any existing server before starting another instance.

## Test on Windows

Ensure port 8080 is free before running the tests.

```bash
ctest --test-dir .extras/WindowsNative/build --output-on-failure
```

The suite runs three checks:

- `windows-shims`: monotonic timers, concurrent timing calls, integer conversion,
  and floating-point formatting using the same assertions as the macOS example.
- `windows-sockets`: real loopback TCP connections exercising shared socket
  ownership, WebSocket and SSE handoffs, automatic Winsock initialization and
  lifetime, buffered reads, disconnects, reconnection, nonblocking reads,
  timeouts, write backpressure, listener
  conflicts and aWOT request processing.
- `windows-http`: starts the built executable on port 8080 and checks HTTP
  status, content type, the Hello World body, missing routes and concurrent
  requests. It also checks startup failure on an occupied port and terminates
  the test server after completion.

CTest bounds each test's runtime so a blocking regression fails the build.
Python uses only its standard library. For an application-only build, configure
with `-DBUILD_TESTING=OFF` to omit Python and the tests.

The [Unit Tests workflow](../../.github/workflows/NativeUnitTests.yml) includes a
`Windows native example` job on `windows-latest`. It builds and **runs** all three
checks on Windows for relevant pushes and pull requests. This is the native
Windows verification; cross-compilation alone only checks that the code builds.

## Cross-check from macOS or Linux

With a MinGW cross-compiler and Wine installed, the same CMake project can build
Windows executables and run its tests under Wine:

```bash
cmake -S .extras/WindowsNative -B .extras/WindowsNative/build-wine -G Ninja \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
  -DCMAKE_CROSSCOMPILING_EMULATOR="$(command -v wine)" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build .extras/WindowsNative/build-wine --parallel 4
ctest --test-dir .extras/WindowsNative/build-wine --output-on-failure
```

Wine provides a useful local check of the Windows executable. Keep the Windows
CI run as well to exercise Microsoft's Winsock implementation.

## Adapter behavior

Client copies share their native `SOCKET`, connection status, and unread
receive buffer. Destroying or reassigning a handle releases only its reference;
the last reference closes the socket. `stop()` closes the connection and clears
buffered input for every copy. Reconnecting a handle closes its old shared
connection and attaches that handle to a new one. Synchronize concurrent access
to copies of the same connection.

The adapter manages `WSAStartup`/`WSACleanup` for each shared connection, closes
sockets with `closesocket`, and keeps reads nonblocking so Arduino/aWOT request
timeouts work. Each shared connection has a 1 KiB receive buffer, including for
raw `request.client()` reads. Listener waits sleep until a connection arrives.

`WsChannel<WindowsClient, N>` and `SseChannel<WindowsClient, N>` can retain these
shared handles. For channel routes, retain clients only when `app.process()`
returns `responseOpen`; pass successful SSE connections to `events.add(client)`.
Let the local client leave scope without calling `stop()` after handing it to
the channel. The Hello World worker deliberately stops every connection, so
adapt its lifecycle before using channel routes.

Like the macOS adapter, outgoing connections accept numeric IPv4 addresses;
DNS and IPv6 connections are not implemented. The example listens on all IPv4
interfaces.

## License

The example follows the aWOT library's MIT license. The bundled ArduinoCore-API
retains its own license notices in [`../ArduinoCore-API`](../ArduinoCore-API/LICENSE).
