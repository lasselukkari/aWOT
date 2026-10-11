---
title: "Running aWOT on macOS"
description: "Build and run an aWOT web server on your Mac with native sockets, without Arduino hardware."
route: "/en/guide/macos.html"
section: "guide"
---

# Running on macOS

Run aWOT on your Mac to develop an HTTP API or browser interface without a
board connected. The native example in `.extras/MacOSNative` provides a socket
adapter for Arduino's `Client` interface, so the same aWOT routes and request
handlers work on macOS.

This builds a regular command-line program with Apple's C++ compiler. You do
not need Arduino IDE, a board package, or a network shield.

For another desktop platform, follow the [native Windows guide](/en/guide/windows.html)
or [native Linux guide](/en/guide/linux.html).

## Get the source

Install Xcode Command Line Tools if they are not already installed:

```bash
xcode-select --install
```

They provide `clang++`, `make`, and Git. Clone the aWOT repository
and enter the native example directory:

```bash
git clone https://github.com/lasselukkari/aWOT.git
cd aWOT/.extras/MacOSNative
```

If you already have an aWOT checkout, open its
`.extras/MacOSNative` directory instead. Use a Git checkout for this example:
`.extras` is omitted from library release archives and Arduino Library Manager
packages.

The ArduinoCore-API sources are bundled in the example. Its String files are
named `WString.h` and `WString.cpp` to avoid a collision with the macOS system
header `string.h`; no separate download or renaming is needed. Keep the example
inside the checkout, since the Makefile also compiles the library from
`../../src`.

## Hello world example

`HelloWorld.cpp` registers a route and handles each connection in its own thread:

```cpp
#include <aWOT.h>

#include <atomic>
#include <thread>

#include "MacOSSocket.h"

MacOSServer server(8080);
App app;
std::atomic<int> activeConnections(0);

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

void handleClient(MacOSClient client) {
  activeConnections++;
  app.process(&client);
  client.stop();
  activeConnections--;
}

bool startServer() {
  Serial.begin(115200);

  app.get("/", &indexCmd);
  if (!server.begin()) return false;

  Serial.println("Server listening on port 8080 (multithreaded)");
  Serial.flush();
  return true;
}

void loop() {
  MacOSClient client = server.available(-1);
  if (client.connected()) {
    std::thread(handleClient, std::move(client)).detach();
  }
}

int main() {
  if (!startServer()) return EXIT_FAILURE;
  while (true) {
    loop();
  }
  return 0;
}
```

## Build and run

From `.extras/MacOSNative`, run:

```bash
make
make run
```

`make` produces the `HelloWorld` executable. `make run` starts it and prints:

```text
Server listening on port 8080 (multithreaded)
```

Leave this terminal running and open <http://localhost:8080/> in your browser.
The page displays **Greetings middle earth!**. The native server listens on
all IPv4 interfaces; `localhost` or `127.0.0.1` connects from the same Mac.

You can also inspect the response from a second terminal:

```bash
curl -i http://127.0.0.1:8080/
```

The response has status `200 OK`, a `Content-Type` of `text/html`, and the HTML
page as its body. Only `GET /` is registered in this example. A request to an
unregistered path returns `404 Not Found`:

```bash
curl -i http://127.0.0.1:8080/missing
```

Press **Control-C** in the server terminal to stop it.

## Use your own routes

Edit `HelloWorld.cpp` to build your application. The `indexCmd` handler writes
the example page; its `Request` and `Response` arguments are the same ones used
on Arduino. Register additional handlers in `startServer()`, before
`server.begin()`, using `app.get()`, `app.post()`, or the other
[routing methods](/en/starter/basic-routing.html).

After changing the file, stop the running server and run `make run` again. The
Makefile rebuilds the changed code before starting the new executable.

The transport changes, but the HTTP application stays familiar: use
[middleware](/en/guide/middleware.html), read
[request data](/en/guide/request-data.html), or serve a browser interface with
the same aWOT API. The compatibility layer supplies core Arduino types and
timing functions; it does not emulate board peripherals such as GPIO or a
Wi-Fi radio. Keep hardware-specific operations separate from the HTTP handlers
when sharing code with a board.

## How the native server works

The [native example source](https://github.com/lasselukkari/aWOT/tree/master/.extras/MacOSNative)
contains these pieces:

| File or directory | Purpose |
| --- | --- |
| `HelloWorld.cpp` | Registers routes, starts the listener, and handles each accepted connection in a worker thread. |
| `MacOSSocket.h` and `MacOSSocket.cpp` | Implement `MacOSServer` and `MacOSClient` with native BSD sockets. |
| `Arduino.h` | Supplies the compatibility layer for timing, formatting, and terminal output through `Serial`. |
| `ArduinoCore-API/` | Provides Arduino types such as `Client`, `Stream`, `Print`, and `String`. |
| `Makefile` | Builds the example, adapter, Arduino API, and aWOT sources together as C++11. |

The main loop waits for connections with `server.available(-1)`. Each accepted
`MacOSClient` is moved into a worker thread, which calls `app.process(&client)`
and then `client.stop()`. The example handles one request per connection and
closes it afterward. See [Connections](/en/guide/connections.html)
before changing that lifecycle.

Route handlers can run concurrently. Register routes before accepting clients,
and synchronize shared mutable application state with a mutex or atomics.
Give each concurrent request its own header-capture buffers and custom request
and response storage, and synchronize access to any shared context.

`MacOSClient` owns its socket and can be moved but cannot be copied. The
`WsChannel` and `SseChannel` helpers store copies of client handles, so this
adapter cannot be substituted directly into those channel examples.

## Run the native checks

Stop the server first, then run:

```bash
make test
```

This builds and runs the socket and compatibility tests, then starts its own
server for an HTTP smoke test. Port **8080 must be free**. A successful run ends
with `Test passed`.

To remove the example executable, test executables, and object files:

```bash
make clean
```

## Troubleshooting

- **The example directory is missing:** use a Git checkout containing
  `.extras/MacOSNative`; release archives omit development files.
- **The server exits at startup:** check whether another process is already
  listening on port 8080. Stop that process before starting the example.
- **You still see the old response:** stop the old process before rebuilding
  and restarting. Recompiling does not replace an already running server.

Continue with [routing](/en/guide/routing.html) to organize a larger application,
or [Hello world on a board](/en/starter/hello-world.html) to use Wi-Fi or Ethernet.
