# Linux Native aWOT Example

Run the aWOT Hello World server on Linux without Arduino hardware. This example
shares the POSIX socket adapter and Arduino compatibility layer with the macOS
example. `LinuxClient` and `LinuxServer` alias those existing classes; the aWOT
library sources run unchanged. Each accepted connection moves into a separate
`std::thread`.

## Prerequisites

- Linux with GCC or Clang supporting C++11
- CMake 3.18 or newer and Make or Ninja
- Python 3 for the HTTP test

On Debian or Ubuntu:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build python3
```

Keep this directory inside a Git checkout containing `.extras/MacOSNative` and
`.extras/WindowsNative`. The build reuses the POSIX adapter and ArduinoCore-API
bundled in the macOS directory; the HTTP test reuses the Python script from the
Windows directory. No separate Arduino installation or download is needed.

## Build and run

From the repository root:

```bash
cmake -S .extras/LinuxNative -B .extras/LinuxNative/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build .extras/LinuxNative/build --parallel 4
./.extras/LinuxNative/build/HelloWorld
```

Visit <http://127.0.0.1:8080/>. The page displays **Greetings middle earth!**.
Stop the server with Ctrl+C. The server always listens on port 8080 and exits
with an error if that port is occupied. Stop any existing server before
starting another instance. The listener binds all IPv4 interfaces.

Configure with `-DBUILD_TESTING=OFF` for an application-only build that omits
Python and the tests. The CMake project requires Linux; use the
[macOS example](../MacOSNative/README.md) or
[Windows example](../WindowsNative/README.md) on those systems.

## Test on Linux

Ensure port 8080 is free before running the tests.

```bash
ctest --test-dir .extras/LinuxNative/build --output-on-failure
```

The suite runs three checks:

- `linux-shims`: monotonic timers, concurrent timing calls, integer conversion,
  and floating-point formatting using the shared POSIX compatibility layer.
- `linux-sockets`: socket ownership, buffered reads, disconnects, reconnection,
  nonblocking reads, read and aWOT timeouts, listener conflicts and waits,
  writes after peer close, and aWOT request processing. It reuses the macOS
  socket tests with POSIX socket pairs and TCP loopback connections.
- `linux-http`: starts the built executable on port 8080 and checks HTTP
  status, content type, the Hello World body, missing routes, and concurrent
  requests. It also checks startup failure on an occupied port and terminates
  the test server after completion.

CTest bounds each test's runtime. Python uses only its standard library. A
successful run reports `100% tests passed, 0 tests failed out of 3`.

The [Unit Tests workflow](../../.github/workflows/NativeUnitTests.yml) includes a
`Linux native example` job on Ubuntu. It builds and runs these checks for
relevant pushes and pull requests. On 11 October 2026, the example built with
GCC 14.4 and passed all three checks in a Debian Linux Docker container using
Python 3.13. The first hosted Linux CI run remains pending until the changes
are pushed; review that job's result at the source revision you use.

## Adapter behavior

`LinuxSocket.h` aliases `MacOSClient` and `MacOSServer` from the shared POSIX
implementation. `Arduino.h` forwards the shared Arduino compatibility header.
CMake compiles that implementation and ArduinoCore-API alongside the aWOT
sources, and links the platform's thread support.

Clients own their socket descriptors and can be moved but cannot be copied.
Reads return promptly when no data is ready, allowing Arduino and aWOT request
timeouts to expire. Each client has a 1 KiB receive buffer, including for raw
`request.client()` reads. Listener waits sleep in `poll` until a connection
arrives. Linux sends use `MSG_NOSIGNAL`, so writing to a disconnected peer
reports failure through the client API without raising `SIGPIPE`.

Outgoing connections accept numeric IPv4 addresses; DNS and IPv6 connections
are not implemented. The `WsChannel` and `SseChannel` helpers copy client
handles, so this move-only adapter cannot be substituted directly into those
channel examples.

Register routes before accepting clients. Synchronize shared mutable state and
provide separate header buffers and custom request/response storage for each
concurrent request. The example processes one request per connection and closes
the client afterward.

## License

The example follows the aWOT library's MIT license. The bundled ArduinoCore-API
retains its own license notices in `../MacOSNative/ArduinoCore-API`.
