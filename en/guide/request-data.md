---
layout: page
title: Reading request data in aWOT
menu: guide
lang: en
---

# Request data

A handler reads the URL, selected headers, and body from `Request`. Query,
form, and route extraction return a boolean; check it before using their
buffers. Capacities include the terminating NUL byte.

<h2 id="headers">Capture headers per request</h2>

aWOT stores only the application headers you ask it to capture. Declare a
contiguous `Request::Header` array and pass it to `process(client, headers)`,
or assign it to `ProcessOptions.headers`. Arrays infer capacities and counts.
Names are compared without regard to case.

`req.get("X-Label")` returns null when that header is absent or was not
captured. A present header with an empty value returns a non-null empty string.
Buffers and presence state are cleared before parsing and after processing.
Read or copy the value inside a handler; it does not survive `process()`.

<h2 id="query-and-forms">Query and form values</h2>

`req.path()` is percent-decoded; after successful parsing, `req.query()` returns
the raw query without the `?`, or an empty string if there was no query. Named
`req.query(name, buffer, capacity)`
decodes the name and value and treats `+` as a space. A literal `+` stays literal
in a path. Invalid percent escapes and decoded NUL anywhere in the request
target are rejected with `400` before routing.

`req.form(name, nameCapacity, value, valueCapacity)` consumes the next
`application/x-www-form-urlencoded` field from the body and decodes its name
and value. It also treats `+` as a space. aWOT does not select this parser based
on Content-Type; your handler chooses when to call it. It is a sequential
parser, rather than a lookup by field name.

A false result can mean a missing query, invalid input, or insufficient space.
An overflow leaves a NUL-terminated partial result, which the example rejects.
Form overflow consumes through the raw `&` delimiter or end of body so a later
call can read the next field. Invalid pointers and nonpositive capacities fail
without touching the buffers.

<h2 id="complete-example">Headers, query, form, context, and custom storage</h2>

This complete ESP32 sketch demonstrates three routes. `GET /inspect?name=Ada`
reads a query and captured header; `POST /form` reads form fields; `PUT /echo`
reads and returns a bounded body. Its per-request context measures processing
time, and custom buffers replace the default URL and output storage.

```arduino
#include <WiFi.h>
#include <aWOT.h>
#include <string.h>

WiFiServer server(80);
App app;

struct RequestState {
  unsigned long started;
  int status;
};

void inspect(Request& req, Response& res) {
  char name[32];
  if (!req.query("name", name, sizeof(name))) {
    res.sendStatus(400);
    return;
  }
  const char* label = req.get("X-Label");
  res.print("Name: ");
  res.println(name);
  res.print("Label: ");
  res.println(label ? label : "(absent)");
}

void readForm(Request& req, Response& res) {
  const char* type = req.get("Content-Type");
  if (!type || strcmp(type, "application/x-www-form-urlencoded") != 0) {
    res.sendStatus(415);
    return;
  }
  if (req.left() <= 0 || req.left() > 128) {
    res.sendStatus(400);
    return;
  }
  // Validate every field before starting the success response.
  unsigned int fields = 0;
  while (req.left() > 0) {
    char name[16];
    char value[32];
    if (!req.form(name, sizeof(name), value, sizeof(value))) {
      res.sendStatus(400);
      return;
    }
    ++fields;
  }
  res.print("Fields received: ");
  res.print(fields);
}

void echo(Request& req, Response& res) {
  if (req.left() <= 0 || req.left() > 128) {
    res.sendStatus(400);
    return;
  }
  char body[128];
  size_t length = req.left();
  if (req.readBytes(body, length) != length || req.left() != 0) {
    res.sendStatus(400);
    return;
  }
  res.set("Content-Type", "application/octet-stream");
  res.write((const uint8_t*)body, length);
}

void finish(Request& req, Response& res) {
  RequestState* state = static_cast<RequestState*>(req.context);
  state->status = res.statusSent();
  Serial.print("HTTP status: ");
  Serial.print(state->status);
  Serial.print("; elapsed ms: ");
  Serial.println(millis() - state->started);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());
  app.get("/inspect", inspect);
  app.post("/form", readForm);
  app.put("/echo", echo);
  app.finally(finish);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client.connected()) {
    char label[32];
    char contentType[48];
    Request::Header headers[] = {
      {"X-Label", label}, {"Content-Type", contentType}
    };
    char url[64];
    uint8_t output[32];
    RequestState state = {millis(), 0};
    App::ProcessOptions options;
    options.headers = headers;
    options.urlBuffer = url;
    options.writeBuffer = output;
    options.context = &state;
    app.process(&client, options);
    client.stop();
  }
}
```

Set your credentials and use the printed address:

```bash
curl --header 'X-Label: example' 'http://192.168.1.140/inspect?name=Ada+Lovelace'
curl --data 'name=Ada&city=Helsinki' http://192.168.1.140/form
curl --request PUT --data-binary 'hello' http://192.168.1.140/echo
```

<h2 id="body">Body lengths and reads</h2>

`req.left()` reports unread body bytes, including pushed-back bytes. Ordinary
`read()` is nonblocking and returns `-1` when no byte is available; inherited
`readBytes()` waits up to the request's Stream timeout and returns the number
actually read. Check both the returned count and the expected body length.
`available()` describes bytes ready now, so it does not prove a complete body
has arrived. `bytesRead()` also counts the request line and headers; it is not
the body length.

Content-Length is decimal, bounded by `INT_MAX`, and allows surrounding spaces
or tabs. Repeated lengths must agree. Comma lists, conflicting lengths, and
every incoming Transfer-Encoding are rejected with `400` and closure. Incoming
chunked bodies are unsupported. Without Content-Length, the body length is
zero. A short or timed-out body needs an application error response; reading
the body after routing does not automatically run `onError`.

Read or discard the complete body before retaining an HTTP connection. On
rejected requests, this example closes the connection. See
[connections](/en/guide/connections.html) and
[JSON](/en/guide/json.html) for structured body parsing.

<h2 id="borrowed-storage">Borrowed storage and context</h2>

`ProcessOptions` uses `ArrayView`: it borrows existing arrays without allocating
or copying them. All names, header buffers, URL/output buffers, and context
must remain alive throughout `process()`. Concurrent calls need separate
storage. The caller owns `context`; handlers receive the same pointer through
`req.context`. Copy any data needed afterward into your own storage.

Arrays infer their sizes. This partial equivalent shows the explicit sizes
needed when your variables are raw pointers; supply their actual capacities
and header count:

```arduino
// Inside a caller that owns these pointers and their storage.
Request::Header capture("X-Label", labelPointer, labelCapacity);
App::ProcessOptions options;
options.urlBuffer = {urlPointer, urlCapacity};
options.writeBuffer = {outputPointer, outputCapacity};
options.headers = {&capture, 1};
options.context = contextPointer;
app.process(&client, options);
```

Null URL or output buffer pointers choose default storage. A custom URL buffer
needs at least two bytes; output capacity must exceed `CHUNK_OVERHEAD`.
Active header entries need a non-null name and buffer with positive capacity.
Invalid options stop the client before any route, error, or final callback.

`req.client()` returns the supplied `Client*`. If you call `process(Stream*)`,
it points to a temporary `StreamClient` adapter that expires on return. Never
retain that pointer, or a `Request`/`Response` pointer, for later use.
