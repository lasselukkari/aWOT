# aWOT

A portable HTTP server library for Arduino, with an API inspired by Express.js.

aWOT is built to work across Arduino platforms, from an Uno to an ESP32, and
on macOS, Windows, and Linux with Arduino-compatible socket adapters. It uses
the standard Arduino `Client` and `Stream` interfaces, so your route handlers
work over Wi-Fi, Ethernet, Bluetooth, or even a serial port.

Requests are Arduino `Stream` objects and responses are `Print` objects. That
means you can pass a request directly to ArduinoJson or an OTA updater, and
write a response with the same methods you already use elsewhere in Arduino.

Build an HTTP API, serve a browser interface, or combine both into a complete
web application. The companion asset tooling turns your frontend build into
gzipped files that mount on the server with one line.

## Get started

Install **aWOT** through your environment's library manager:

- **Arduino IDE:** Open [Library Manager](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-installing-a-library/), search for `aWOT`, and choose **Install**.
- **PlatformIO:** Open [Library Manager](https://docs.platformio.org/en/latest/librarymanager/index.html), search for `aWOT`, and add it to your project.

This complete example uses an ESP32 with its Arduino board package installed:

```cpp
#include <WiFi.h>
#include <aWOT.h>

WiFiServer server(80);
App app;

void hello(Request& req, Response& res) {
  res.print("Hello World!");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println(WiFi.localIP());

  app.get("/", hello);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client.connected()) {
    app.process(&client);
  }
}
```

Set your Wi-Fi credentials, upload the sketch, and open the address printed in
the Serial Monitor at 115200 baud. The browser displays `Hello World!`.

## Your transport, the same application

aWOT handles HTTP using the transport you provide. Any implementation of
Arduino's `Client` or `Stream` interfaces can be passed to `app.process()`.

| Transport | Processing a request |
| --- | --- |
| Wi-Fi or Ethernet | `app.process(&client)` |
| Serial, with a peer sending HTTP | `app.process(&Serial)` |
| Native macOS sockets | `app.process(&client)` using the supplied adapter |
| Native Windows sockets | `app.process(&client)` using the Winsock adapter |
| Native Linux sockets | `app.process(&client)` using the POSIX adapter |

See the [Uno Ethernet example](examples/Ethernet/Ethernet.ino),
[ESP32/ESP8266 Wi-Fi example](examples/WiFi/WiFi.ino),
[native macOS example](.extras/MacOSNative/README.md),
[native Windows example](.extras/WindowsNative/README.md), and
[native Linux example](.extras/LinuxNative/README.md). The native adapters
are available in a Git checkout under `.extras/`.

## Routes and middleware

Register handlers for HTTP methods and URL patterns. Middleware can inspect a
request and call `req.next()` to continue to the next matching handler.

```cpp
void logRequest(Request& req, Response& res) {
  Serial.println(req.path());
  req.next();
}

void greet(Request& req, Response& res) {
  char name[32];
  if (!req.route("name", name, sizeof(name))) {
    return res.sendStatus(400);
  }
  res.print("Hello, ");
  res.print(name);
}
```

Add these registrations to `setup()`, before `server.begin()`:

```cpp
app.use(logRequest);
app.get("/hello/:name", greet);
```

Visiting `/hello/Arduino` returns `Hello, Arduino`. Handlers run in registration
order; returning without `req.next()` finishes dispatch.

Use `get`, `head`, `post`, `put`, `patch`, `del`, and `options` for method-specific
routes. Group routes with `Router`, mount them with `app.use()`, and customize
missing routes with `app.notFound()`.

## Works with existing Arduino libraries

`Request` inherits from `Stream`, and `Response` inherits from `Print`. Libraries
that use these interfaces can read the request body and write the response
directly.

For example, install [ArduinoJson](https://arduinojson.org/) and include
`<ArduinoJson.h>` in your sketch. Inside a handler, read JSON from the request
and write it to the response:

```cpp
JsonDocument doc;
if (deserializeJson(doc, req)) {
  return res.sendStatus(400);
}

res.set("Content-Type", "application/json");
serializeJson(doc, res);
```

The body goes straight to the parser without an intermediate string. This
snippet uses ArduinoJson 7, whose
[parser accepts `Stream`](https://arduinojson.org/v7/api/json/deserializejson/)
and [serializer accepts `Print`](https://arduinojson.org/v7/api/json/serializejson/).

The same interface makes firmware uploads straightforward on supported boards.
Once the updater is initialized, stream uploaded firmware from the request:

```cpp
Update.writeStream(req);
```

The [OTA example](examples/OTAUpdate/OTAUpdate.ino) shows the complete update
flow, including setup, error handling, and restarting the device.

## Build the frontend and firmware together

Develop your browser interface with the frontend tools you prefer. During local
development, configure your frontend development server to proxy API requests
to the Arduino. Relative requests such as `fetch('/led')` then work during
development and when the interface is served by the device.

The companion [awot-static tool](https://github.com/lasselukkari/awot-scripts#awot-static)
compresses your frontend build with gzip and generates `StaticFiles.h`. The
generated routes include the content type, content length, and encoding headers
for each asset.

Include the generated header:

```cpp
#include "StaticFiles.h"
```

Then mount the files in `setup()` after registering your API routes:

```cpp
app.use(staticFiles());
```

Build the frontend, generate the assets, and upload the sketch. Your device now
serves both the browser interface and its HTTP API.

The [full-stack tutorial](https://awot.net/en/guide/tutorial.html) walks through
the workflow with a browser-controlled LED.

## Explore further

| Example | What it demonstrates |
| --- | --- |
| [Authentication](examples/AuthMiddleware/AuthMiddleware.ino) | Request headers and middleware |
| [WebSockets](examples/WebSockets/WebSockets.ino) | A browser chat interface |
| [Server-sent events](examples/ServerSentEvents/ServerSentEvents.ino) | Streaming updates to the browser |
| [File server](examples/FileServer/FileServer.ino) | Serving files from storage |
| [Member callbacks](examples/MemberFunction/MemberFunction.ino) | Organizing handlers in classes |
| [Request context](examples/RequestContext/RequestContext.ino) | Sharing application data with handlers |
| [Keep-alive](examples/KeepAlive/KeepAlive.ino) | Reusing HTTP connections |

See the [documentation website](https://awot.net/) for guides, the API reference,
and migration instructions. The [source contracts](src/README.md) cover buffer
boundaries, parsing behavior, and transport handling.

## License

Licensed under [MIT](LICENSE). Based on Webduino, Copyright 2009–2014 Ben
Combee, Ran Talbott, Christopher Lee, Martin Lormes, Francisco M Cuenca-Acuna.
