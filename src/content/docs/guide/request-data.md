---
title: "Reading request data in aWOT"
description: "Read request headers, query values, route parameters, and HTTP request bodies."
route: "/en/guide/request-data.html"
section: "guide"
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

<span id="query-and-forms"></span>
<h2 id="query-values">Query values</h2>


`req.path()` is percent-decoded; after successful parsing, `req.query()` returns
the raw query without the `?`, or an empty string if there was no query. Named
`req.query(name, buffer, capacity)`
decodes the name and value and treats `+` as a space. A literal `+` stays literal
in a path. Invalid percent escapes and decoded NUL anywhere in the request
target are rejected with `400` before routing.

A false result can mean a missing query value, invalid input, or insufficient
space. An overflow leaves a NUL-terminated partial result, which the example
rejects. Invalid pointers and nonpositive capacities fail without touching the
buffer.

<h2 id="form-values">Form values</h2>

`req.form(name, nameCapacity, value, valueCapacity)` consumes the next
`application/x-www-form-urlencoded` field from the body and decodes its name
and value. It also treats `+` as a space. aWOT does not select this parser based
on Content-Type; your handler chooses when to call it. It is a sequential
parser, rather than a lookup by field name.

A false result can mean invalid input or insufficient space. An overflow leaves
a NUL-terminated partial result, which the example rejects, and consumes through
the raw `&` delimiter or end of body so a later call can read the next field.
Invalid pointers and nonpositive capacities fail without touching the buffers.

<h2 id="complete-example">Headers, query, form, context, and custom storage</h2>


This complete ESP32 sketch demonstrates three routes. `GET /inspect?name=Ada`
reads a query and captured header; `POST /form` reads form fields; `PUT /echo`
reads and returns a bounded body. Its per-request context measures processing
time, and custom buffers replace the default URL and output storage.
The form and echo routes require a known, nonzero Content-Length of at most
128 bytes; they reject chunked requests because the total length is unknown.

```cpp
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


For Content-Length bodies, `req.left()` reports unread body bytes, including
pushed-back bytes. For chunked bodies, it returns `-1` until the zero chunk and
final empty trailer line have been consumed, even if pushback contains bytes.
It then returns the pushback count or zero. Use `req.left() != 0` when a loop
must also handle chunked bodies, and handle failed reads so incomplete input
does not leave the loop spinning.

Ordinary `read()` is nonblocking and returns `-1` when no byte is available;
the buffer overload can read across chunks. Inherited `readBytes()` waits up
to the request's Stream timeout and returns the number actually read. Both
decode chunked payload without allocating a body buffer. Check returned counts
and body completion. `available()` reports ready payload from the current
chunk or remaining Content-Length, plus pushback, capped at `INT_MAX`; it does
not prove a complete body has arrived. Both `left()` and `available()` can
consume ready chunk framing without waiting for input. `bytesRead()` counts
transport bytes, including the request line, headers, chunk framing, and
trailers, once; it is not the decoded body length.

Content-Length is decimal, bounded by `INT_MAX`, and allows surrounding spaces
or tabs. Repeated lengths must agree. HTTP/1.1 accepts a single
`Transfer-Encoding: chunked` field. Each chunk can declare up to `UINT32_MAX`
bytes, and the complete body can span multiple chunks. Extensions are ignored;
trailers are consumed and discarded without changing captured headers. Comma
lists or conflicting Content-Length values, Content-Length combined with
Transfer-Encoding, repeated or unsupported transfer codings, and transfer
coding on HTTP/1.0 are rejected with `400` and closure. Without either framing
header, the body length is zero.

Malformed chunk framing closes the connection. If no response status has been
sent when routing finishes, aWOT invokes `onError` with `400` and supplies its
usual fallback response; it does not replace an already-started response.
Check short or timed-out reads yourself: inherited Stream helpers do not
guarantee an `onError` call for incomplete input.

Read or discard the complete body before retaining an HTTP connection. An
incomplete chunked body prevents both keep-alive reuse and an open-response
handoff. On rejected requests, this example closes the connection. See
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

```cpp
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
