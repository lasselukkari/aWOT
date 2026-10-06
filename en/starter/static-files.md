---
layout: page
title: Serving static files in aWOT 4
menu: starter
lang: en
---

# Serving static files in aWOT 4

The published `awot-static` command in **awot-scripts 3.1.2** converts a directory
of browser assets into gzipped flash data and a `Router` in `StaticFiles.h`.
Its output has been compiled with aWOT 4.0.0. The converter's package version is
independent of the library version; install the library with Arduino Library
Manager and create the sketch manually as shown in
[Create a project](/en/starter/generator.html).

In an existing Vite frontend, install the pinned converter:

```bash
npm install --save-dev --save-exact awot-scripts@3.1.2
mkdir WebServer
```

Add these fields to `package.json`, keeping your existing dependencies and
scripts. Vite's `react-ts` template uses `tsc -b && vite build` for `build`:

```json
{
  "scripts": {
    "build": "tsc -b && vite build",
    "build:firmware": "npm run build && awot-static"
  },
  "awot-static": {
    "sources": "./dist",
    "indexFile": "index.html",
    "sketchDir": "./WebServer",
    "exclude": ["*.map"]
  }
}
```

Keep the generated Vite TypeScript configuration and `package-lock.json`.
Use Node.js **22.12 or newer** for this workflow. Run:

```bash
npm run build:firmware
```

The build creates `dist/` and then `WebServer/StaticFiles.h`. Each generated
handler sets `Content-Type`, `Content-Encoding: gzip`, `Content-Length`, cache
headers, and writes the compressed bytes with `res.writeP()`. The browser
receives and decompresses those assets. The configured index is served at `/`;
other files use their generated paths. This converter does not supply a generic
client-side router fallback for arbitrary URLs.

Create `WebServer/WebServer.ino` with this complete ESP32 sketch. Set your Wi-Fi
credentials before uploading:

```arduino
#include <WiFi.h>
#include <aWOT.h>
#include "StaticFiles.h"

WiFiServer server(80);
App app;

void health(Request& req, Response& res) {
  res.set("Content-Type", "text/plain");
  res.print("ok");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());
  app.get("/api/health", health);
  app.use(staticFiles());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
```

Mount `staticFiles()` **once in setup**, after the API routes. The generated
function returns a pointer to a static router, which outlives its use by `app`.
Generate the header again whenever frontend assets change, then rebuild and
upload the firmware. The assets occupy flash; their size must fit your board's
application partition. This workflow does not measure or promise an AVR memory
budget.

The [full stack tutorial](/en/guide/tutorial.html) adds a Vite development proxy,
frontend API calls, and LED control. The
[3.x static-file instructions](/en/3x/starter/static-files.html) are archived for
older projects.
