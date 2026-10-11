---
title: "Running aWOT on Windows"
description: "Build, run, and test an aWOT web server on Windows with MinGW and native Winsock sockets, without Arduino hardware."
route: "/en/guide/windows.html"
section: "guide"
---

# Running on Windows

Run aWOT on Windows to develop an HTTP API or browser interface without a board
connected. The native example in `.extras/WindowsNative` ports the
[macOS example](/en/guide/macos.html) to Winsock and implements Arduino's `Client`
interface. The aWOT routes and request handlers use the same library sources.

This builds a regular Windows command-line program with MinGW GCC. You do not
need Arduino IDE, a board package, or a network shield. The instructions below
target 64-bit Intel/AMD Windows with the MSYS2 UCRT64 toolchain.

For a Linux computer, follow the [native Linux guide](/en/guide/linux.html).

## Get the tools and source

Install [MSYS2](https://www.msys2.org/) and open its **UCRT64** terminal. Install
Git, the compiler, CMake, Ninja, and Python:

```bash
pacman -S --needed git mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-python
```

Use the UCRT64 terminal for the following commands. The MinGW packages build
native Windows executables; see [MSYS2's CMake instructions](https://www.msys2.org/docs/cmake/)
for this toolchain setup. The example supports MinGW GCC. The bundled Arduino
API uses GNU attributes and has not been adapted for MSVC.

Clone the aWOT repository and stay in the repository root:

```bash
git clone https://github.com/lasselukkari/aWOT.git
cd aWOT
```

If you already have a checkout containing `.extras/WindowsNative`, use its root
directory instead. Use a Git checkout for this example: `.extras` is omitted
from library release archives and Arduino Library Manager packages.

Keep the example inside the checkout. CMake compiles aWOT from `src/` and reuses
the ArduinoCore-API sources bundled in `.extras/MacOSNative/ArduinoCore-API`.
That directory is needed on Windows too; no separate Arduino installation or
download is required.

## Hello world example

`HelloWorld.cpp` registers a route and handles each connection in its own thread:

```cpp
/*
  HelloWorld.cpp - aWOT example using native Windows Winsock sockets

  Run HelloWorld.exe, then visit http://localhost:8080/.
*/

#include <aWOT.h>

#include <cstdlib>
#include <thread>
#include <utility>

#include "WindowsSocket.h"

App app;

void indexCmd(Request&, Response& res) {
  res.set("Content-Type", "text/html");
  res.print("<html>\n");
  res.print("<head>\n");
  res.print("<title>Hello World!</title>\n");
  res.print("</head>\n");
  res.print("<body>\n");
  res.print("<h1>Greetings middle earth!</h1>\n");
  res.print("</body>\n");
  res.print("</html>");
}

void handleClient(WindowsClient client) {
  app.process(&client);
  client.stop();
}

int main() {
  Serial.begin(115200);
  app.get("/", &indexCmd);
  WindowsServer server(8080);
  if (!server.begin()) return EXIT_FAILURE;
  Serial.println("Server listening on port 8080 (multithreaded)");
  Serial.flush();

  for (;;) {
    // Wait in select until a connection arrives, with no idle polling loop.
    WindowsClient client = server.available(-1);
    if (client.connected()) {
      std::thread(handleClient, std::move(client)).detach();
    }
  }
}
```

## Build and run

From the repository root in the UCRT64 terminal, run:

```bash
cmake -S .extras/WindowsNative -B .extras/WindowsNative/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build .extras/WindowsNative/build --parallel 4
./.extras/WindowsNative/build/HelloWorld.exe
```

CMake builds the example and its tests as C++11 programs. The server prints:

```text
Server listening on port 8080 (multithreaded)
```

Leave this terminal running and open <http://127.0.0.1:8080/> in your browser.
The page displays **Greetings middle earth!**. The server listens on all IPv4
interfaces; `127.0.0.1` connects from the same Windows computer.

You can also inspect the response from a second terminal. In PowerShell, use
`curl.exe`:

```powershell
curl.exe -i http://127.0.0.1:8080/
curl.exe -i http://127.0.0.1:8080/missing
```

The first request returns `200 OK`, a `Content-Type` of `text/html`, and the
HTML page. Only `GET /` is registered, so `/missing` returns `404 Not Found`.
Press **Control-C** in the server terminal to stop it.

The executable can also run from PowerShell or Explorer after building. CMake
links the GCC, C++, and thread runtimes statically, so the UCRT64 terminal is
only needed for the build commands.

## Use your own routes

Edit `.extras/WindowsNative/HelloWorld.cpp` to build your application. Register
additional handlers in `main()`, before `server.begin()`, using `app.get()`,
`app.post()`, or the other [routing methods](/en/starter/basic-routing.html).

After editing, stop the running server, build again with
`cmake --build .extras/WindowsNative/build --parallel 4`, and start the executable.
Use [middleware](/en/guide/middleware.html) and
[request data](/en/guide/request-data.html) with the same aWOT API as on a board.
The compatibility layer supplies Arduino types, timing, formatting, and terminal
output through `Serial`. It does not emulate GPIO or a Wi-Fi radio. Keep
hardware-specific operations separate when sharing handlers with a board.

## How the native server works

The [Windows native source](https://github.com/lasselukkari/aWOT/tree/master/.extras/WindowsNative)
contains these pieces:

| File or directory | Purpose |
| --- | --- |
| `HelloWorld.cpp` | Registers routes, starts the listener on port 8080, and moves each connection into a worker thread. |
| `WindowsSocket.h` and `WindowsSocket.cpp` | Implement `WindowsServer` and `WindowsClient` with Winsock. |
| `Arduino.h` and `Arduino.cpp` | Supply the compatibility layer for timing, formatting, and terminal output. |
| `../MacOSNative/ArduinoCore-API/` | Supplies Arduino types such as `Client`, `Stream`, `Print`, and `String`. |
| `CMakeLists.txt` | Builds the adapter, example, aWOT sources, and tests, and links Winsock. |
| `tests/` | Tests the socket adapter and the running HTTP server. |

`server.available(-1)` waits for connections without an idle polling loop. Each
accepted `WindowsClient` is moved into a worker thread, which calls
`app.process(&client)` and then `client.stop()`. The example handles one request
per connection and closes it afterward. See
[Connections](/en/guide/connections.html) before changing that lifecycle.

Route handlers can run concurrently. Register routes before accepting clients,
and synchronize shared mutable state with a mutex or atomics. Give each
concurrent request its own header-capture buffers and custom request and response
storage, and synchronize access to any shared context.

`WindowsClient` owns its `SOCKET` and closes it with `closesocket`. It can be
moved but cannot be copied. The adapter manages `WSAStartup` and `WSACleanup`;
each live socket retains Winsock initialization, including clients that outlive
their listener. Reads use nonblocking sockets and a 1 KiB per-client receive
buffer, including for raw `request.client()` reads, so Arduino and aWOT can
apply their read timeouts. Writes wait for backpressure to clear within the
client's timeout.

Outgoing `WindowsClient::connect()` calls accept numeric IPv4 addresses. DNS
names and IPv6 connections are not implemented. The `WsChannel` and `SseChannel`
helpers store copies of client handles, so this adapter cannot be substituted
directly into those channel examples.

## Run the Windows checks

From the repository root in the UCRT64 terminal, run:

```bash
ctest --test-dir .extras/WindowsNative/build --output-on-failure
```

The suite runs three tests:

- **windows-shims:** checks timers, concurrent timing calls, integer conversion,
  and floating-point formatting using the same assertions as the macOS example.
- **windows-sockets:** uses real loopback TCP connections to check ownership,
  Winsock lifetime, buffered and nonblocking reads, disconnects, reconnection,
  timeouts, write backpressure, listener conflicts, and aWOT request processing.
- **windows-http:** starts the built executable on port 8080 and checks status
  codes, content type, the Hello World body, missing routes, and concurrent
  requests. It also checks that startup fails on an occupied port, then
  terminates the test server.

Stop the example server before running the tests; port **8080 must be free**.
Each test has a bounded runtime. A successful run reports
`100% tests passed, 0 tests failed out of 3`. Python is only needed for the
HTTP test; to build the application without tests, add
`-DBUILD_TESTING=OFF` to the CMake configuration command.

The [native Unit Tests workflow](https://github.com/lasselukkari/aWOT/blob/master/.github/workflows/NativeUnitTests.yml)
includes a **Windows native example** job on `windows-latest`. It builds and
runs these checks on Windows for relevant pushes and pull requests. A Windows
run exercises Microsoft's Winsock implementation; cross-compiling only
establishes that the code builds. Wine can provide an additional local check
from macOS or Linux using the commands in the
[example README](https://github.com/lasselukkari/aWOT/blob/master/.extras/WindowsNative/README.md#cross-check-from-macos-or-linux).

As of 11 October 2026, all three checks pass under Wine; the first native
Windows CI run is pending. Check the Windows job's result before considering
the port verified on Windows.

## Troubleshooting

- **The example directory is missing:** use a Git checkout containing
  `.extras/WindowsNative`; release archives omit `.extras`.
- **CMake rejects the toolchain:** open the MSYS2 UCRT64 terminal and install the
  `mingw-w64-ucrt-x86_64-*` packages above. This project requires Windows MinGW.
  If you configured with another compiler, configure again using a new build
  directory so the previous CMake compiler selection is not reused.
- **Python is missing during configuration:** install
  `mingw-w64-ucrt-x86_64-python`, or configure with `-DBUILD_TESTING=OFF` for an
  application-only build.
- **The server exits at startup:** read the reported Winsock error and check
  whether another process uses port 8080. Stop that process before starting
  the example. Windows exclusive binding can keep a recently used port
  reserved while closed TCP connections finish; wait before restarting.
- **You still see the old response:** stop the old process before rebuilding
  and restarting. Recompiling does not replace an already running server.

Continue with [routing](/en/guide/routing.html) to organize a larger application,
or [Hello world on a board](/en/starter/hello-world.html) to use Wi-Fi or Ethernet.
