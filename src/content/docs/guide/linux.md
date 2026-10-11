---
title: "Running aWOT on Linux"
description: "Build, run, and test an aWOT web server on Linux with native POSIX sockets, without Arduino hardware."
route: "/en/guide/linux.html"
section: "guide"
---

# Running on Linux

Run aWOT on Linux to develop an HTTP API or browser interface without a board
connected. The example in `.extras/LinuxNative` shares the POSIX socket adapter
and Arduino compatibility layer with the [macOS example](/en/guide/macos.html).
`LinuxClient` and `LinuxServer` are aliases for those existing client and server
classes; the aWOT routes and request handlers use the same library sources.

This builds a regular Linux command-line program with GCC or Clang. You do not
need Arduino IDE, a board package, or a network shield. For a Windows computer,
follow the [native Windows guide](/en/guide/windows.html).

## Get the tools and source

Install a C++11 compiler, CMake **3.18 or newer**, Ninja, Git, and Python 3.
Python is needed for the HTTP test. On Debian or Ubuntu, install the tools and
`curl` with:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build python3 git curl
```

Clone the aWOT repository and stay in the repository root:

```bash
git clone https://github.com/lasselukkari/aWOT.git
cd aWOT
```

If you already have a checkout containing `.extras/LinuxNative`, use its root
directory instead. Use a Git checkout: `.extras` is omitted from library release
archives and Arduino Library Manager packages.

Keep `.extras/MacOSNative` and `.extras/WindowsNative` in the checkout too. The
Linux build reuses the POSIX adapter and ArduinoCore-API bundled in the macOS
directory, and its HTTP test uses the shared Python script in the Windows
directory. No separate Arduino download or installation is required.

## Hello world example

`HelloWorld.cpp` registers a route and moves each connection into its own thread:

```cpp
/*
  HelloWorld.cpp - aWOT example using native Linux POSIX sockets

  Run ./HelloWorld, then visit http://localhost:8080/.
*/

#include <aWOT.h>

#include <cstdlib>
#include <thread>
#include <utility>

#include "LinuxSocket.h"

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

void handleClient(LinuxClient client) {
  app.process(&client);
  client.stop();
}

int main() {
  Serial.begin(115200);
  app.get("/", &indexCmd);
  LinuxServer server(8080);
  if (!server.begin()) return EXIT_FAILURE;
  Serial.println("Server listening on port 8080 (multithreaded)");
  Serial.flush();

  for (;;) {
    // Wait in poll until a connection arrives, with no idle polling loop.
    LinuxClient client = server.available(-1);
    if (client.connected()) {
      std::thread(handleClient, std::move(client)).detach();
    }
  }
}
```

## Build and run

From the repository root, run:

```bash
cmake -S .extras/LinuxNative -B .extras/LinuxNative/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build .extras/LinuxNative/build --parallel 4
./.extras/LinuxNative/build/HelloWorld
```

CMake builds the example and its tests as C++11 programs. The server prints:

```text
Server listening on port 8080 (multithreaded)
```

Leave this terminal running and open <http://127.0.0.1:8080/> in your browser.
The page displays **Greetings middle earth!**. The server listens on all IPv4
interfaces; `127.0.0.1` connects from the same Linux computer.

Inspect the responses from another terminal:

```bash
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.1:8080/missing
```

The first request returns `200 OK`, a `Content-Type` of `text/html`, and the
HTML page. Only `GET /` is registered, so `/missing` returns `404 Not Found`.
Press **Control-C** in the server terminal to stop it.

## Use your own routes

Edit `.extras/LinuxNative/HelloWorld.cpp`. Register additional handlers in
`main()`, before `server.begin()`, using `app.get()`, `app.post()`, or the other
[routing methods](/en/starter/basic-routing.html). Stop the running server,
rebuild with `cmake --build .extras/LinuxNative/build --parallel 4`, and start it
again after editing.

Use [middleware](/en/guide/middleware.html) and
[request data](/en/guide/request-data.html) with the same aWOT API as on a board.
The compatibility layer supplies Arduino types, timing, formatting, and terminal
output through `Serial`. Keep operations using board peripherals separate when
sharing handlers with a board.

## How the native server works

The [Linux native source](https://github.com/lasselukkari/aWOT/tree/master/.extras/LinuxNative)
contains the application and CMake build. `LinuxSocket.h` aliases the shared
`MacOSClient` and `MacOSServer` classes as `LinuxClient` and `LinuxServer`;
`Arduino.h` forwards the shared Arduino compatibility header. CMake compiles
the existing POSIX implementation and bundled ArduinoCore-API with aWOT.

`server.available(-1)` waits in `poll` until a connection arrives. Each accepted
client moves into a worker thread, which calls `app.process(&client)` and then
`client.stop()`. The example handles one request per connection and closes it
afterward. See [Connections](/en/guide/connections.html) before changing that
lifecycle.

Register routes before accepting clients. Synchronize shared mutable state with
a mutex or atomics, give each concurrent request its own header-capture buffers
and custom request and response storage, and synchronize access to shared
context.

Clients own their socket descriptors and can be moved but cannot be copied.
Socket reads return promptly when no data is available, allowing Arduino and
aWOT read timeouts to expire. Each client has a 1 KiB receive buffer, including
for raw `request.client()` reads. Linux sends suppress `SIGPIPE` with
`MSG_NOSIGNAL`, so writing to a disconnected peer reports failure through the
client API.

Outgoing connections accept numeric IPv4 addresses; DNS and IPv6 connections
are not implemented. The `WsChannel` and `SseChannel` helpers store copies of
client handles, so this adapter cannot be substituted directly into those
channel examples.

## Run the Linux checks

From the repository root, run:

```bash
ctest --test-dir .extras/LinuxNative/build --output-on-failure
```

The suite runs three tests:

- **linux-shims:** checks timers, concurrent timing calls, integer conversion,
  and floating-point formatting using the shared POSIX compatibility layer.
- **linux-sockets:** checks socket ownership, buffered and nonblocking reads,
  disconnects, reconnection, timeouts, writes after peer close, listener
  conflicts, and aWOT request processing with POSIX and loopback TCP sockets.
- **linux-http:** starts the built executable on port 8080 and checks status
  codes, content type, the Hello World body, missing routes, and concurrent
  requests. It also checks startup failure on an occupied port and terminates
  the test server.

Stop the example server before running the tests; port **8080 must be free**.
Each test has a bounded runtime. A successful run reports
`100% tests passed, 0 tests failed out of 3`. Configure with
`-DBUILD_TESTING=OFF` to build only the application without Python or the tests.

The [native Unit Tests workflow](https://github.com/lasselukkari/aWOT/blob/master/.github/workflows/NativeUnitTests.yml)
includes a **Linux native example** job that builds and runs these checks on
Ubuntu. On 11 October 2026, the example built with GCC **14.4** and passed all
three checks in a Debian Linux Docker container using Python **3.13**. The first
hosted Linux CI run remains pending until the library changes are pushed; review
that job's result at the source revision you use. The exact program above also
compiled, linked, and passed its HTTP smoke test in that Linux environment.
Compilation on macOS and building this documentation do not establish Linux
runtime support.

## Troubleshooting

- **The example directory is missing:** use a Git checkout containing
  `.extras/LinuxNative`; release archives omit `.extras`.
- **CMake rejects the platform:** this project requires Linux. Use the
  [macOS guide](/en/guide/macos.html) or [Windows guide](/en/guide/windows.html)
  on those systems.
- **Python is missing during configuration:** install Python 3 or configure
  with `-DBUILD_TESTING=OFF` for an application-only build.
- **The server exits at startup:** check whether another process uses the
  fixed port 8080. Stop that process before starting the example.
- **You still see the old response:** stop the old process before rebuilding
  and restarting. Recompiling does not replace an already running server.

Continue with [routing](/en/guide/routing.html) to organize a larger application,
or [Hello world on a board](/en/starter/hello-world.html) to use Wi-Fi or Ethernet.
