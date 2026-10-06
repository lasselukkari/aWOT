---
layout: page
title: Arduino Full Stack Tutorial
menu: guide
lang: en
---

# Arduino Full Stack Tutorial

## Deploying a React app on the ESP32 with Vite and TypeScript

Build a browser interface that switches an LED on and off, then serve both the
interface and its HTTP API from an ESP32. During development, Vite serves the
React app on your computer and forwards API requests to the board. For deployment,
we compile the TypeScript, build the frontend, and embed the resulting files in
the firmware as compressed assets.

This tutorial uses **aWOT 4.0.0**, React, and Vite's `react-ts` template. The
firmware uses `App` and closes each connection after processing its request.

The complete project is available in
[LedBlink](https://github.com/lasselukkari/LedBlink/tree/ece7d3e1ab0a57c977742468af619a201c199959).
Its README explains how to build and upload the finished application if you
want to try it before following the walkthrough.

1. [Development environment](#environment)
2. [Project setup](#setup)
3. [Back-end development](#back-end)
4. [Front-end development](#front-end)
5. [Building and deploying the project](#build)

<h3 id="environment">1. Development environment</h3>

You need an ESP32 development board with a controllable LED, a USB cable, and a
Wi-Fi network that both your computer and board can join. This example uses a
regular GPIO LED; check your board's pin and whether the LED is active high or
active low. Boards with addressable RGB LEDs need their own LED driver.

Install:

- [Node.js](https://nodejs.org/en/download), version **22.12 or newer**. Vite
  requires Node.js 20.19+ or 22.12+; a recent Node.js LTS release meets these
  requirements.
- [Arduino IDE](https://www.arduino.cc/en/software).
- The [Arduino ESP32 board package](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html).
  Select your board and serial port in the Arduino IDE.
- **aWOT 4.0.0**. Open Library Manager in the Arduino IDE, search for `aWOT`,
  select version **4.0.0**, and choose **Install**.

Check your Node.js installation:

```bash
node --version
npm --version
```

The [Vite getting started guide](https://vite.dev/guide/) explains its Node.js
requirements and project templates.

<h3 id="setup">2. Project setup</h3>

Create the React and TypeScript project with Vite:

```bash
npm create vite@9.2.1 led-blink -- --template react-ts --no-interactive
cd led-blink
npm install
```

Install the asset converter and create the Arduino sketch directory:

```bash
npm install --save-dev --save-exact awot-scripts@3.1.2
mkdir BlinkServer
```

We use the published `awot-static` command from
[awot-scripts 3.1.2](https://github.com/lasselukkari/awot-scripts#awot-static)
to generate assets compatible with aWOT 4. The tool's version is independent of
the library version. Create the firmware manually as shown below so that it uses
the installed v4 library.

The files you will edit are:

```text
led-blink/
├── BlinkServer/
│   ├── BlinkServer.ino
│   └── StaticFiles.h       (generated when building for the board)
├── src/
│   ├── App.tsx
│   ├── App.css
│   ├── index.css
│   └── main.tsx            (keep Vite's generated entry point)
├── index.html
├── package.json
├── package-lock.json
├── tsconfig.json
├── tsconfig.app.json
├── tsconfig.node.json
└── vite.config.ts
```

Keep the generated TypeScript configuration files and commit `package-lock.json`
with your project. Use `npm ci` to install the locked dependencies on another
computer. Vite's TypeScript template includes a build step that checks types
before producing the frontend assets.

<h3 id="back-end">3. Back-end development</h3>

The API uses one byte of plain text to represent the LED state:

| Request | Behavior |
| --- | --- |
| `GET /led` | Return `0` when the LED is off or `1` when it is on. |
| `PUT /led` with body `0` or `1` | Set the LED and return its new state. |
| `PUT /led` with any other body | Return HTTP 400 without changing the LED. |

Create `BlinkServer/BlinkServer.ino` with this complete sketch. Set your Wi-Fi
credentials and the LED pin and polarity for your board:

```arduino
#include <WiFi.h>
#include <aWOT.h>

const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";

const int LED_PIN = 2;
const bool LED_ACTIVE_LOW = false;

WiFiServer server(80);
App app;
bool ledOn = false;

void writeLed() {
  const bool high = LED_ACTIVE_LOW ? !ledOn : ledOn;
  digitalWrite(LED_PIN, high ? HIGH : LOW);
}

void readLed(Request& req, Response& res) {
  res.set("Content-Type", "text/plain; charset=utf-8");
  res.set("Cache-Control", "no-store");
  res.print(ledOn ? '1' : '0');
}

void updateLed(Request& req, Response& res) {
  if (req.left() != 1) {
    res.sendStatus(400);
    return;
  }

  char value;
  if (req.readBytes(&value, 1) != 1 || (value != '0' && value != '1')) {
    res.sendStatus(400);
    return;
  }

  ledOn = value == '1';
  writeLed();
  readLed(req, res);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  writeLed();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  Serial.print("Open http://");
  Serial.println(WiFi.localIP());

  app.get("/led", readLed);
  app.put("/led", updateLed);
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

`Request` is an Arduino `Stream`. Its `readBytes()` method waits for the body
byte, so we can check the read succeeded before changing the LED. `req.left()`
checks that the request declares exactly one byte. `Response` is a `Print`:
set headers before writing the body with `print()`.

Returning from a handler without calling `req.next()` finishes route dispatch
in v4. `app.process()` finalizes an ordinary response, and the caller owns the
connection. This sketch calls `client.stop()` after every request; it does not
request connection reuse or leave responses open.

Open `BlinkServer.ino` in Arduino IDE and upload it. Open Serial Monitor at
**115200 baud** and record the printed IP address. Replace `192.168.1.227` in
the following commands with your board's address:

```bash
curl --fail http://192.168.1.227/led
curl --fail -X PUT -H 'Content-Type: text/plain' --data-binary '1' http://192.168.1.227/led
curl --fail -X PUT -H 'Content-Type: text/plain' --data-binary '0' http://192.168.1.227/led
```

The first command reads the state. The next two switch the LED on and off.
To check validation, this request should return `400 Bad Request` and leave the
LED unchanged:

```bash
curl -i -X PUT --data-binary 'invalid' http://192.168.1.227/led
```

<h3 id="front-end">4. Front-end development</h3>

Configure Vite to forward `/led` requests to the ESP32. Replace `vite.config.ts`
with the following, using the IP address from Serial Monitor:

```typescript
import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

export default defineConfig({
  plugins: [react()],
  base: '/',
  server: {
    proxy: {
      '/led': {
        target: 'http://192.168.1.227',
        changeOrigin: true,
      },
    },
  },
})
```

This is Vite's [development proxy](https://vite.dev/config/server-options.html#server-proxy).
Restart the development server after changing the configuration or the board's
IP address. The frontend uses relative URLs such as `fetch('/led')`: Vite proxies
them during development, and they go directly to the ESP32 when the board serves
the built app.

Replace `src/App.tsx` with:

```tsx
import { useEffect, useState } from 'react'
import './App.css'

async function requestLed(options?: RequestInit): Promise<boolean> {
  const response = await fetch('/led', { cache: 'no-store', ...options })
  if (!response.ok) {
    throw new Error(`Request failed: HTTP ${response.status}`)
  }

  const state = await response.text()
  if (state !== '0' && state !== '1') {
    throw new Error('The board returned an invalid LED state')
  }
  return state === '1'
}

export default function App() {
  const [ledOn, setLedOn] = useState<boolean | null>(null)
  const [busy, setBusy] = useState(false)
  const [error, setError] = useState('')

  useEffect(() => {
    let ignore = false

    async function loadLed() {
      try {
        const state = await requestLed()
        if (!ignore) setLedOn(state)
      } catch (cause: unknown) {
        if (!ignore) {
          setError(cause instanceof Error ? cause.message : 'Cannot reach the board')
        }
      }
    }

    void loadLed()
    return () => {
      ignore = true
    }
  }, [])

  async function toggleLed() {
    if (ledOn === null || busy) return
    setBusy(true)
    setError('')

    try {
      const state = await requestLed({
        method: 'PUT',
        headers: { 'Content-Type': 'text/plain' },
        body: ledOn ? '0' : '1',
      })
      setLedOn(state)
    } catch (cause: unknown) {
      setError(cause instanceof Error ? cause.message : 'Cannot reach the board')
    } finally {
      setBusy(false)
    }
  }

  return (
    <main>
      <h1>ESP32 LED control</h1>
      <p>
        {ledOn === null
          ? error ? 'LED state unavailable' : 'Reading LED state…'
          : `The LED is ${ledOn ? 'on' : 'off'}.`}
      </p>
      <button
        type="button"
        disabled={ledOn === null || busy}
        onClick={toggleLed}
      >
        {busy ? 'Updating…' : ledOn ? 'Turn off' : 'Turn on'}
      </button>
      {error && <p role="alert">{error}. Check the board connection and reload to retry.</p>}
    </main>
  )
}
```

TypeScript models the initial state as `null` until the board responds. We also
validate the response at runtime: a successful HTTP status alone does not prove
that the body is a valid LED state. The button is disabled while reading the
initial state or sending an update, and the display changes after the board
confirms the new value. The effect sets an `ignore` flag during cleanup so a
late response or error cannot update state after that effect has been cleaned
up, including during React's development checks.

The API calls use `async`/`await` with `try`/`catch` for errors. The effect starts
an inner async function so its callback can return the cleanup function that
sets the flag. The request can finish, but its result is ignored after cleanup.

Replace `src/index.css` to remove the template's page styles:

```css
:root {
  font-family: system-ui, sans-serif;
  color: #16343b;
  background: #f2f7f7;
}

body {
  margin: 0;
}

button {
  font: inherit;
}
```

Replace `src/App.css` with:

```css
main {
  max-width: 30rem;
  margin: 4rem auto;
  padding: 2rem;
  text-align: center;
}

button {
  padding: 0.75rem 1.5rem;
  border: 0;
  border-radius: 0.5rem;
  color: white;
  background: #007d85;
  cursor: pointer;
}

button:disabled {
  opacity: 0.5;
  cursor: wait;
}

[role='alert'] {
  color: #a12323;
}
```

Keep `src/main.tsx` as generated. Replace the root `index.html` with:

```html
<!doctype html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>ESP32 LED control</title>
  </head>
  <body>
    <div id="root"></div>
    <script type="module" src="/src/main.tsx"></script>
  </body>
</html>
```

Delete the template assets in `public/` and `src/assets/` to keep unused files out
of the deployed bundle.
Start Vite:

```bash
npm run dev
```

Open the local URL printed by Vite, usually `http://localhost:5173`. You should
see the LED's current state and be able to change it with the button. Edits to
the frontend appear through Vite's development server without uploading the
firmware again. If a request fails, check that the board is running and that its
address matches the proxy target.

<h3 id="build">5. Building and deploying the project</h3>

Vite writes the production frontend to **`dist/`**. Merge the following entries
into the existing `package.json`, keeping the template's dependencies and other
scripts. The JSON below shows the relevant fields, rather than a replacement for
the entire file:

```json
{
  "scripts": {
    "dev": "vite",
    "build": "tsc -b && vite build",
    "build:firmware": "npm run build && awot-static"
  },
  "awot-static": {
    "sources": "./dist",
    "indexFile": "index.html",
    "sketchDir": "./BlinkServer",
    "exclude": ["*.map"]
  }
}
```

Run this command from the `led-blink` project directory:

```bash
npm run build:firmware
```

The command checks TypeScript, builds the frontend, then generates
`BlinkServer/StaticFiles.h` from `dist/`. Check that the converter completes
successfully and the header contains the current build before uploading.
The generated handlers serve gzip-compressed files with their content type,
encoding, and compressed content length. The index is served at `/`, and Vite's
JavaScript and CSS files are served under `/assets/`.

Add this include after `<aWOT.h>` in `BlinkServer.ino`:

```arduino
#include "StaticFiles.h"
```

In `setup()`, mount the generated router after the API routes and before starting
the server:

```arduino
app.get("/led", readLed);
app.put("/led", updateLed);
app.use(staticFiles());
server.begin();
```

Mount the router once during setup. The `base: '/'` setting in `vite.config.ts`
keeps asset URLs aligned with these routes. This app has one page at `/`; adding
client-side routes later also requires handling those paths in the firmware.

Compile and upload `BlinkServer.ino` again, then open the board's address, such as
`http://192.168.1.227/`. You can now close Vite: the ESP32 serves both the interface
and the API. Verify that `/` displays the app, `/led` returns its state, and the
button still switches the physical LED.

You can also inspect the compressed index response from the command line:

```bash
curl --fail --compressed -i http://192.168.1.227/
```

After changing the frontend, run `npm run build:firmware` and upload again.
After changing only firmware code, compile and upload the sketch. The built web
assets consume program flash along with the library and sketch; check the
Arduino compiler's size report against your board's application partition.

Add `BlinkServer/StaticFiles.h` to `.gitignore` if you generate it during your
build process. Keep the source files and dependency lockfile so you can rebuild
the assets when needed.
