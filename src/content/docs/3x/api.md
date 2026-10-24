---
title: "aWOT 3.x - API Reference"
description: "Archived aWOT 3 API reference for Application, Request, Response, and Router."
route: "/en/3x/api.html"
section: "archive"
apiVersion: "3x"
---

# aWOT 3.x API

> **aWOT 3.x archive.** This page describes the previous API.
> Read the [aWOT 4 reference](/en/4x/api.html) or the
> [migration guide](/en/guide/migrating-to-v4.html) for current projects.
<h2 id="app">Application</h2>

The Application class is used for mounting Middleware to Routers and to process the incoming HTTP requests.

##### Example

```cpp
#include <WiFi.h>
#include <aWOT.h>

WiFiServer server(80);
Application app;

void index(Request &req, Response &res) {
  res.print("Hello Wolrd!");
}

void setup() {
  Serial.begin(115200);

  WiFi.begin("ssid", "password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.get("/", &index);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }
}
```

<h3 id='app-methods'>Methods</h3>


<h3 id='app-del'>Application::del</h3>

<h4 class='variant'>void del(const char* path, Router::Middleware* middleware);</h4>

<h4 class='variant'>void del(Router::Middleware* middleware);</h4>


Routes HTTP DELETE requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example
```cpp
void deleteSomething(Request &req, Response &res) {
  res.sendStatus(204);
}

app.del("/delete", &deleteSomething);
```


<h3 id='app-finally'>Application::finally</h3>

<h4 class='variant'>void finally(Router::Middleware* middleware);</h4>


This method is like any other app.METHOD(), except it executed always as the last middleware. The middleware is called even if the request does not match any routes or the request has been ended. There can be only one final middleware. If the method is called more than once the previous calls are ineffective.


<h3 id='app-get'>Application::get</h3>

<h4 class='variant'>void get(const char* path, Router::Middleware* middleware);</h4>

<h4 class='variant'>void get(Router::Middleware* middleware);</h4>


Routes HTTP GET requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example
```cpp
void index(Request &req, Response &res) {
  res.print("Hello World!");
}

app.get("/index.html", &index);
```


<h3 id='app-head'>Application::head</h3>

<h4 class='variant'>void head(const char* path, Router::Middleware* middleware);</h4>

<h4 class='variant'>void head(Router::Middleware* middleware);</h4>


Routes HTTP HEAD requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example
```cpp
void index(Request &req, Response &res) {
  res.set("etag", "5ebd65ed-2b957");
  res.status(200);
}

app.head("/index.html", &index);
```


<h3 id='app-header'>Application::header</h3>

<h4 class='variant'>void header(const char* name, char* buffer, int bufferLength);</h4>


When request gets to the first middleware all headers have been already processed. To make header lookup available inside a Middleware with the Request::get method we need to specify the headers we are interested in advance.

The header name matching is case-insensitive and the buffer needs to have space for null termination. If the header value does not fit the buffer the server will automatically send a `431 Request Header Fields Too Large` response.

If the the same header name is defined multiple times in the request the values will be concatenated together using commas.

##### Example

```cpp
char contentType[100];

void headers(Request &req, Response &res) {
  char * type = req.get("Content-Type");
}

app.header("Content-Type", contentType, 100);
app.use(&headers);
```


<h3 id='app-notFound'>Application::notFound</h3>

<h4 class='variant'>void notFound(Router::Middleware* middleware);</h4>


By defualt the server send a generic 404 - Not Found page if none of the routes match to the request. The notFound mehod can be use to set up a custon middleware handler to be executed instead.


<h3 id='app-options'>Application::options</h3>

<h4 class='variant'>void options(const char* path, Router::Middleware* middleware);</h4>

<h4 class='variant'>void options(Router::Middleware* middleware);</h4>


Routes HTTP OPTIONS requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example

```cpp
void cors(Request &req, Response &res) {
  res.set("Access-Control-Allow-Origin", "*");
  res.set("Access-Control-Allow-Methods", "GET, HEAD");
  res.sendStatus(204);
}

app.options("/js/app.js", &cors);
```


<h3 id='app-patch'>Application::patch</h3>

<h4 class='variant'>void patch(const char* path, Router::Middleware* middleware);</h4>

<h4 class='variant'>void patch(Router::Middleware* middleware);</h4>


Routes HTTP PATCH requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example

```cpp
void patchSomething(Request &req, Response &res) {
  char body[100];
  req.readBytes(type, 100);
}

app.patch("/example", &patchSomething);
```


<h3 id='app-post'>Application::post</h3>

<h4 class='variant'>void post(const char* path, Router::Middleware* middleware);</h4>

<h4 class='variant'>void post(Router::Middleware* middleware);</h4>


Routes HTTP POST requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example

```cpp
void form(Request &req, Response &res) {
  char name[10];
  char value[10];

  while (req.left()) {
    if (!req.form(name, 10, value, 10)) {
      return res.sendStatus(400);
    }

    res.print(name);
    res.print(":");
    res.println(value);
  }
}

app.post("/form", &form);
```


<h3 id='app-process'>Application::process</h3>

<h4 class='variant'>void process(Stream *client, void *context = NULL);</h4>

<h4 class='variant'>void process(Stream *client, char *buffer, int bufferLength, void *context = NULL);</h4>

<h4 class='variant'>void process(Client *client, void *context = NULL);</h4>

<h4 class='variant'>void process(Client *client, char *buffer, int bufferLength, void *context = NULL);</h4>


The method processes a connected client. Call this method in yout main loop when ever you have connected client available.

The Client class is an Arduino defined base class so we can process requests from multiple different Stream implementations such as EthernetClient or WifiClient. For debugging purposes it is event possible to use the serial port.


The basic of version of process uses the internal buffer for storing the url of the request. The max length is defined in `SERVER_URL_BUFFER_SIZE`. The second overloaded version works exactly the same way but the buffer for storing the url of the HTTP request is given as a argument.

The context can be used for passing arbitrary data to the middleware functions.

##### Example

```cpp
void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }
}
```


<h3 id='app-put'>Application::put</h3>

<h4 class='variant'>void put(const char* path, Router::Middleware* middleware);</h4>

<h4 class='variant'>void put(Router::Middleware* middleware);</h4>


Routes HTTP PUT requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example

```cpp
void putSomething(Request &req, Response &res) {
  char body[100];
  req.readBytes(type, 100);
}

app.put("/example", &putSomething);
```


<h3 id='app-use'>Application::use</h3>


<h4 class='variant'>void use(const char* path, Middleware* middleware);</h4>

<h4 class='variant'>void use(Router::Middleware* middleware);</h4>

<h4 class='variant'>void use(const char* path, Router* router);</h4>

<h4 class='variant'>void use(Router * router);</h4>


This method is like any standard app.METHOD() methods, except it matches all HTTP verbs and routes. The overloaded version mounts a Router instance to the application.

If the path parameter is not specified it's set to NULL the middleware or router will be executed on all requests.

##### Example Middleware

```cpp
void logRequest(Request &req, Response &res) {
  Serial.print(millis());
  Serial.print(": ");

  switch (req.method()) {
    case  Request::GET: {
        Serial.print("GET ");
        break;
      }
    case  Request::POST: {
        Serial.print("POST ");
        break;
      }
    case  Request::PUT: {
        Serial.print("PUT ");
        break;
      }
    case  Request::PATCH: {
        Serial.print("GET ");
        break;
      }
    case  Request::DELETE: {
        Serial.print("DELETE ");
        break;
      }
    default: {}
  }

  Serial.println(req.path());
}

app.use(&logRequest);
```

##### Example Router

```cpp
Application app;
Router cats;

void looooong(Request &req, Response &res) {
  res.print("looooong cat is long!");
}

void ceiling(Request &req, Response &res) {
  res.print("ceiling cat is watching you debug!");
}

void nyannyan(Request &req, Response &res) {
  for (int i = 0; i < 100; i++) {
      res.print("nyan ");
  }
}

cats.get("/long", &looooong);
cats.get("/ceiling", &ceiling);
cats.get("/nyan", &nyannyan);

app.use("/cats", &cats);
```

<h2 id="req">Request</h2>


<h3 id='req-types'>Types</h3>


<h3 id='req-methodType'>Request::MethodType</h3>

<h4 class='variant'>enum MethodType {GET, HEAD, POST, PUT, DELETE, PATCH, OPTIONS, ALL, USE}</h4>


The incoming HTTP request method type. HEAD requests are converted to GET requests but only the headers are sent to the client. The method types ALL and USE are used when defining middleware handlers to a Router.


<h3 id='req-variables'>Variables</h3>


<h3 id='req-context'>Request::context</h3>

<h4 class='variant'>void* context;</h4>


The request context pointer can be used for providing arbitrary data to the middleware chain. The pointer can be set or modified by any middlwares and the initial value can be provided as parameters to the [Application:process](/en/3x/api.html#app-process) function.

##### Example
```cpp
struct context {
  IPAddress ip;
};

void index(Request &req, Response &res) {
  context* ctx = (context*)req.context;
  res.print("Your IP is: ");
  res.println(ctx->ip);
}

context ctx = { ip: client.remoteIP() };

app.process(&client, &ctx);
```


<h3 id='req-methods'>Methods</h3>


<h3 id='req-available'>Request::available</h3>

<h4 class='variant'>int available();</h4>


Gets the number of bytes available in the stream. This is only for bytes that have already arrived.


<h3 id='req-availableForWrite'>Request::availableForWrite</h3>

<h4 class='variant'>int availableForWrite();</h4>


Returns the number of bytes (characters) available for writing in the response buffer without blocking the write operation.

This function has been exposed from the response class trough the request so that the request can be used as a [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/).


<h3 id='req-bytesRead'>Request::bytesRead</h3>

<h4 class='variant'>int bytesRead();</h4>


Returns the number of bytes read so far from the incoming stream. This includes the full HTTP request with headers etc.


<h3 id='req-find'>Request::find</h3>

<h4 class='variant'>bool find(char target);</h4>

<h4 class='variant'>bool find(char target, int length);</h4>


Reads data from the stream until the target is found. The function returns true if target is found or false if timed out.


<h3 id='req-findUntil'>Request::findUntil</h3>

<h4 class='variant'>req.findUntil(char target, char terminal);</h4>


Reads data from the stream until the target string of given length or terminator string is found, it times out, or it reaches the end of the request.

The function returns true if target string is found, false if timed out.


<h3 id='req-flush'>Request::flush</h3>

<h4 class='variant'></h4>


Flushes outgoing reposen buffer and waits until all outgoing data in buffer have been sent. This is called internally before the client is disconnected at the end of the request.

This function has been exposed from the response class trough the request so that the request can be used as a [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/).


<h3 id='req-form'>Request::form</h3>

<h4 class='variant'>bool form(char *name, int nameLength, char *value, int valueLength);</h4>


Reads the next key value pair from the request body. The function expects the request content type to be as application/x-www-form-urlencoded.

Returns true if the there was a value in the body. Returns false if there is no name value pair to read or the content were too long for the buffers.

##### Example
```cpp
char name[10];
char value[100];

if(!req.form(name, 10, value, 100)){
  return res.sendStatus(400);
}
```


<h3 id='req-get'>Request::get</h3>

<h4 class='variant'>char * get(const char *name);</h4>


Returns a header value by name. Only header values defined for reading with Application::header() will be available. The matching is case insensitive. If the the same header name is defined multiple times in the request the values will be concatenated together using commas.

##### Example
```cpp
char contentType[100];
void headers(Request &req, Response &res) {
  char * type = req.get("Content-Type");
  // => "text/plain"
}

app.header("Content-Type", contentType, 100);
app.use(&headers);
```


<h3 id='req-left'>Request::left</h3>

<h4 class='variant'>int left();</h4>


Returns the number bytes left to read in the request body. The initial value is taken from the Content-Length header.

##### Example
```cpp
while(req.left() && !req.timedout()){
  res.write(req.read());
}
```


<h3 id='req-method'>Request::method</h3>

<h4 class='variant'> MethodType method();</h4>


Returns the MethodType type of the incoming http request.

##### Example
```cpp
void noDelete(Request &req, Response &res) {
  if (req.method() == Request::DELETE) {
    res.sendStatus(405);
  }
}

app.all("/nodelete", &noDelete);
```


<h3 id='req-parseFloat'>Request::parseFloat</h3>

<h4 class='variant'>float parseFloat();</h4>

<h4 class='variant'>float parseFloat(lookahead LookaheadMode);</h4>

<h4 class='variant'>float parseFloat(lookahead LookaheadMode, ignore char);</h4>


Returns the first valid floating point number from the current position. The function is terminated by the first character that is not a floating point number. The function terminates if it times out or it reaches the end of the request.

The lookahead defines the mode used to look ahead in the stream for a floating point number.

Possilbe lookahead mode values are:

* SKIP_ALL: all characters other than a minus sign, decimal point, or digits are ignored when scanning the stream for a floating point number. This is the default mode.
* SKIP_NONE: Nothing is skipped, and the stream is not touched unless the first waiting character is valid.
* SKIP_WHITESPACE: Only tabs, spaces, line feeds, and carriage returns are skipped.

The ignore char is used to skip the indicated char in the search. Used for example to skip thousands divider.


<h3 id='req-parseInt'>Request::parseInt</h3>

<h4 class='variant'>long parseInt();</h4>

<h4 class='variant'>long parseInt(lookahead LookaheadMode);</h4>

<h4 class='variant'>long parseInt(lookahead LookaheadMode, ignore char);</h4>


Returns the first valid (long) integer number from the current position.

In particular:
* Parsing stops when no characters have been read for a configurable time-out value, a non-digit is read, or it reaches the end of the request.
* If no valid digits were read when the time-out occurs, 0 is returned.

The lookahead defines the mode used to look ahead in the stream for an integer.

Possilbe lookahead mode values are:

* SKIP_ALL: all characters other than digits or a minus sign are ignored when scanning the stream for an integer. This is the default mode.
* SKIP_NONE: Nothing is skipped, and the stream is not touched unless the first waiting character is valid.
* SKIP_WHITESPACE: Only tabs, spaces, line feeds, and carriage returns are skipped.

The ignore char is used to skip the indicated char in the search. Used for example to skip thousands divider.


<h3 id='req-path'>Request::path</h3>

<h4 class='variant'>char * path();</h4>


Returns a null terminated string containing the path part of the request URL.

##### Example
```cpp
// example.com/users?sort=desc
req.path()
// => "/users"
```


<h3 id='req-peek'>Request::peek</h3>

<h4 class='variant'>int peek();</h4>


Returns the next byte of incoming data without removing it from the internal buffer. Successive calls to peek() will return the same character, as will the next call to read().


<h3 id='req-print'>Request::print</h3>

<h4 class='variant'>size_t print(const __FlashStringHelper *);</h4>

<h4 class='variant'>size_t print(const String &);</h4>

<h4 class='variant'>size_t print(const char[]);</h4>

<h4 class='variant'>size_t print(char);</h4>

<h4 class='variant'>size_t print(unsigned char, int base);</h4>

<h4 class='variant'>size_t print(int, int base);</h4>

<h4 class='variant'>size_t print(unsigned int, int base);</h4>

<h4 class='variant'>size_t print(long, int base);</h4>

<h4 class='variant'>size_t print(unsigned long, int base);</h4>

<h4 class='variant'>size_t print(double, int = 2);</h4>

<h4 class='variant'>size_t print(const Printable&);</h4>


Prints data as human-readable ASCII text. This command can take many forms. Numbers are printed using an ASCII character for each digit. Floats are similarly printed as ASCII digits, defaulting to two decimal places. Bytes are sent as a single character. Characters and strings are sent as is.

An optional second parameter specifies the base (format) to use; permitted values are BIN(binary, or base 2), OCT(octal, or base 8), DEC(decimal, or base 10), HEX(hexadecimal, or base 16). For floating point numbers, this parameter specifies the number of decimal places to use.


This function has been exposed from the response class trough the request so that the request can be used as a [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/).


<h3 id='req-println'>Request::println</h3>

<h4 class='variant'>size_t println(const __FlashStringHelper *);</h4>

<h4 class='variant'>size_t println(const String &s);</h4>

<h4 class='variant'>size_t println(const char[]);</h4>

<h4 class='variant'>size_t println(char);</h4>

<h4 class='variant'>size_t println(unsigned char, int = DEC);</h4>

<h4 class='variant'>size_t println(int, int = DEC);</h4>

<h4 class='variant'>size_t println(unsigned int, int = DEC);</h4>

<h4 class='variant'>size_t println(long, int = DEC);</h4>

<h4 class='variant'>size_t println(unsigned long, int = DEC);</h4>

<h4 class='variant'>size_t println(double, int = 2);</h4>

<h4 class='variant'>size_t println(const Printable&);</h4>

<h4 class='variant'>size_t println(void);</h4>


Prints data as human-readable ASCII text followed by a carriage return character (ASCII 13, or '\r') and a newline character (ASCII 10, or '\n'). This command takes the same forms as reponse.print().

This function has been exposed from the response class trough the request so that the request can be used as a [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/).


<h3 id='req-push'>Request::push</h3>

<h4 class='variant'>void push(uint8_t ch);</h4>


Puts a byte back to the read buffer. Undo of read(). If the internal buffer gets full the last byte is replaced over and over again. The internal buffer size is defined in the SERVER_PUSHBACK_BUFFER_SIZE value.


<h3 id='req-query'>Request::query</h3>

<h4 class='variant'>char * query();</h4>

<h4 class='variant'>bool query(const char *name, char *buffer, int bufferLength);</h4>


Without any parameters the method returns a null terminated string containing the query part of the request URL.

The parametrized version reads the request parameter defined by the name to the buffer. The buffer must have space for null termination. Returns true if the parameter was found and the buffer had enough space for the value. Returns false if the parameter is not found or the value was too long for the buffer.

##### Example
```cpp
/cats?type=lolcat
req.query();
// => "type=lolcat"

char type[64];
req.query("type", type, 64);
// => "lolcat"
```


<h3 id='req-read'>Request::read</h3>

<h4 class='variant'>int read();</h4>

<h4 class='variant'>int read(uint8_t* buf, size_t size);</h4>


Reads a byte from the incoming stream. Returns -1 if no data is available.

The buffered version returns the number of bytes read and -1 in case of error.


<h3 id='req-readBytes'>Request::readBytes</h3>

<h4 class='variant'>int readBytes(uint8_t *buffer, int bufferLength);</h4>


Read characters from the request into a buffer. The function terminates if the determined length has been read, it times out, or it reaches the end of the request.

The functionn returns the number of bytes placed in the buffer. A 0 means no valid data was found.

##### Example
```cpp
byte buffer[100];

if(!req.readBytes(buffer, 100)){
  return res.sendStatus(400);
}
```


<h3 id='req-readString'>Request::readString</h3>

<h4 class='variant'>String readString();</h4>


Reads characters from a stream into a String. The function terminates if it times out or it reaches the end of the request.


<h3 id='req-route'>Request::route</h3>

<h4 class='variant'>bool route(const char *name, char *buffer, int bufferLength);</h4>

<h4 class='variant'>    bool route(int number, char *buffer, int bufferLength);</h4>


The firt version of the method reads the route parameter defined by name to the buffer. The buffer must have space for null termination. Returns true if the parameter was found and the buffer had enough space for the value. Returns false if the parameter is not found or the value was too long for the buffer.

The second overload works exaclty the same way but it uses an index number for the look up.

##### Example
```cpp
// HTTP GET /cats/lolcat
void routeParams(Request &req, Response &res) {
  char catId[64];
  req.route("catId", catId, 64);
  // => "lolcat"
}

void setup() {
  app.get("/cats/:catId", &routeParams);
}
```


<h3 id='req-setTimeout'>Request::setTimeout</h3>

<h4 class='variant'>void setTimeout(unsigned long timeout)</h4>


Sets the maximum milliseconds to wait for stream data. The default timeout is 1000 milliseconds.


<h3 id='req-stream'>Request::stream</h3>

<h4 class='variant'>Stream* req.stream();</h4>


Returns a pointer to the raw client stream. You should not normally need to use this. Reading from the returned stream will not add to the the result of bytesRead() and you need to take care of request timeouts and watch out for not reading over the request end.


<h3 id='req-write'>Request::write</h3>

<h4 class='variant'>size_t write(const char *str)</h4>

<h4 class='variant'>size_t write(String str)</h4>

<h4 class='variant'>size_t write(const uint8_t *buffer, size_t lenght)</h4>


Writes data to the response. To send the characters representing the digits of a number use the print() function instead.

This function has been exposed from the response class trough the request so that the request can be used as a [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/).

<h2 id="res">Response</h2>


<h3 id='res-methods'>Methods</h3>


<h3 id='res-availableForWrite'>Response::availableForWrite</h3>

<h4 class='variant'>int availableForWrite();</h4>


Returns the number of bytes (characters) available for writing in the response buffer without blocking the write operation.


<h3 id='res-beginHeaders'>Response::beginHeaders</h3>

<h4 class='variant'>void beginHeaders();</h4>


Calling the function will send the headers set by [Response:set](/en/3x/api.html#res-set) or [Response:setDefaults](/en/3x/api.html#res-setDefaults). The function does not terminate the headers and enables additional headers to be sent by writing them manually to the response. Finalize the headers by calling [Response:endHeaders](/en/3x/api.html#res-endHeaders).

##### Example

```cpp
res.set("Content-Encoding", "gzip");
res.beginHeaders();
res.write("Some-Header: value\r\n");
res.endHeaders();
```


<h3 id='res-bytesSent'>Response::bytesSent</h3>

<h4 class='variant'>int bytesSent();</h4>


Returns the number of bytes written so far to the outgoing response. This includes the full HTTP request with headers. Not just the bytes written to the body.


<h3 id='res-end'>Response::end</h3>

<h4 class='variant'>void end();</h4>


After calling end no further middleware functions will be executed.

##### Example
```cpp
void auth(Request &req, Response &res) {
  char * authHeader = req.get("Authorization");

  if (strcmp(authHeader, "Basic c3VwZXI6YWRtaW4=") != 0) { // super:admin in base64
    res.set("WWW-Authenticate", "Basic realm=\"Secret Area\"");
    res.sendStatus(401);
    res.end();
  }
}
```


<h3 id='res-ended'>Response::ended</h3>

<h4 class='variant'>bool ended();</h4>


Returns true if request has been ended.


<h3 id='res-endHeaders'>Response::endHeaders</h3>

<h4 class='variant'>void endHeaders();</h4>


Finalizes the headers sending when it has been initialized with [Response:beginHeaders](/en/3x/api.html#res-beginHeaders).


<h3 id='res-flush'>Response::flush</h3>

<h4 class='variant'>void flush();</h4>


Flushes outgoing buffer and waits until all outgoing data in buffer have been sent. This is called internally before the client is disconnected at the end of the request.


<h3 id='res-get'>Response::get</h3>

<h4 class='variant'>const char * get(const char *name);</h4>


Returns the HTTP response header specified by name if it has been previously set. The match is case-insensitive.

##### Example
```cpp
res.get('Content-Type');
// => "text/plain"
```


<h3 id='res-headersSent'>Response::headersSent</h3>

<h4 class='variant'>bool headersSent();</h4>


Returns true if the response headers have been already sent. After headers have been sent all writes will go to the request body. Calling any write method will cause headers to be sent automatically.


<h3 id='res-print'>Response::print</h3>

<h4 class='variant'>size_t print(const __FlashStringHelper *);</h4>

<h4 class='variant'>size_t print(const String &);</h4>

<h4 class='variant'>size_t print(const char[]);</h4>

<h4 class='variant'>size_t print(char);</h4>

<h4 class='variant'>size_t print(unsigned char, int base);</h4>

<h4 class='variant'>size_t print(int, int base);</h4>

<h4 class='variant'>size_t print(unsigned int, int base);</h4>

<h4 class='variant'>size_t print(long, int base);</h4>

<h4 class='variant'>size_t print(unsigned long, int base);</h4>

<h4 class='variant'>size_t print(double, int = 2);</h4>

<h4 class='variant'>size_t print(const Printable&);</h4>


Prints data as human-readable ASCII text. This command can take many forms. Numbers are printed using an ASCII character for each digit. Floats are similarly printed as ASCII digits, defaulting to two decimal places. Bytes are sent as a single character. Characters and strings are sent as is.

An optional second parameter specifies the base (format) to use; permitted values are BIN(binary, or base 2), OCT(octal, or base 8), DEC(decimal, or base 10), HEX(hexadecimal, or base 16). For floating point numbers, this parameter specifies the number of decimal places to use.


<h3 id='res-println'>Response::println</h3>

<h4 class='variant'>size_t println(const __FlashStringHelper *);</h4>

<h4 class='variant'>size_t println(const String &s);</h4>

<h4 class='variant'>size_t println(const char[]);</h4>

<h4 class='variant'>size_t println(char);</h4>

<h4 class='variant'>size_t println(unsigned char, int = DEC);</h4>

<h4 class='variant'>size_t println(int, int = DEC);</h4>

<h4 class='variant'>size_t println(unsigned int, int = DEC);</h4>

<h4 class='variant'>size_t println(long, int = DEC);</h4>

<h4 class='variant'>size_t println(unsigned long, int = DEC);</h4>

<h4 class='variant'>size_t println(double, int = 2);</h4>

<h4 class='variant'>size_t println(const Printable&);</h4>

<h4 class='variant'>size_t println(void);</h4>


Prints data as human-readable ASCII text followed by a carriage return character (ASCII 13, or '\r') and a newline character (ASCII 10, or '\n'). This command takes the same forms as reponse.print().


<h3 id='res-printP'>Response::printP</h3>

<h4 class='variant'>void printP(const char *string);</h4>

<h4 class='variant'>void printP(const unsigned char *string);</h4>


Output a string stored in the program memory, usually defined with the P macro. Using this will save you from wasting your precious RAM for static strings.


<h3 id='res-sendStatus'>Response::sendStatus</h3>

<h4 class='variant'>void sendStatus(int code);</h4>


Send a full HTTP response matching to the status code.

##### Example

```cpp
res.sendStatus(404);
 // is equivalent to
res.status(404);
res.print(“Not Found”);
```


<h3 id='res-set'>Response::set</h3>

<h4 class='variant'>void set(const char* name, const char* value);</h4>


Sets the response’s HTTP header field to the value. Existing value will be overridden by the new value if the name matches. The match is case-insensitive. The max number of headers is defined in SERVER_MAX_HEADERS.

##### Example

```cpp
res.set("Content-Type", "text/plain");
```


<h3 id='res-setDefaults'>Response::setDefaults</h3>

<h4 class='variant'>void setDefaults();</h4>


The function is normally called internally when the first call to writing the body happens. If `Content-Type` header has not been set it will be set to `text/plain`. If `Connnection` header has been set to `keep-alive` and `Content-Length` has not been set the `Transfer-Encoding` will set to `chunked`. If `Connection` header has been not set to `keep-alive` it will set it to `close`.


<h3 id='res-status'>Response::status</h3>

<h4 class='variant'>void status(int code);</h4>


Sends the initial status line of the HTTP the response. After calling the function new headers can be still set to the response before first write method is called.

##### Example

```cpp
res.status(404);
res.print("Pagina no encontrada");
```


<h3 id='res-statusSent'>Response::statusSent</h3>

<h4 class='variant'>int statusSent();</h4>


Returns the http status code of the reponse if it has been set.


<h3 id='res-write'>Response::write</h3>

<h4 class='variant'>size_t write(const char *str)</h4>

<h4 class='variant'>size_t write(String str)</h4>

<h4 class='variant'>size_t write(const uint8_t *buffer, size_t lenght)</h4>


Writes data to the response. To send the characters representing the digits of a number use the print() function instead.


<h3 id='res-writeP'>Response::writeP</h3>

<h4 class='variant'>void writeP(const unsigned char *data, size_t length);</h4>


Writes buffer stored in the program memory to the response.

<h2 id="router">Router</h2>


A `Router` is an isolated instance of middleware and routes. You can think of it
as a "mini-application," capable only of performing middleware and routing
functions. Every aWOT application has a built-in default router. Routers can also be nested.

##### Example
```cpp
Application app;
Router cats;

void looooong(Request &req, Response &res) {
  res.print("looooong cat is long!");
}

void ceiling(Request &req, Response &res) {
  res.print("ceiling cat is watching you debug!");
}

void nyannyan(Request &req, Response &res) {
  for (int i = 0; i < 100; i++) {
      res.print("nyan ");
  }
}

void setup() {
  // other setup

  cats.get("/long", &looooong);
  cats.get("/ceiling", &ceiling);
  cats.get("/nyan", &nyannyan);

  app.use("/cats", &cats);
}
```


<h3 id='router-types'>Types</h3>


<h3 id='router-Middleware'>Router::Middleware()</h3>

<h4 class='variant'>void Middleware(Request& request, Response& response);</h4>


The type definition for the middleware handler functions.


<h3 id='router-methods'>Methods</h3>


<h3 id='router-Router'>Router::Router</h3>

<h4 class='variant'>Router();</h4>


Router constructor does not take any parameters.


<h3 id='router-del'>Router::del()</h3>

<h4 class='variant'>void del(const char* urlPattern, Middleware* middleware);</h4>

<h4 class='variant'>void del(Middleware* middleware);</h4>


Routes a DELETE request to a Middleware when the request url matches the url pattern. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).


<h3 id='router-get'>Router::get</h3>

<h4 class='variant'>void get(const char* urlPattern, Middleware* middleware);</h4>

<h4 class='variant'>void get(Middleware* middleware);</h4>


Routes a GET request to a Middleware when the request url matches the url pattern. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).


<h3 id='router-head'>Router::head</h3>

<h4 class='variant'>void head(const char* urlPattern, Middleware* middleware);</h4>

<h4 class='variant'>void head(Middleware* middleware);</h4>


Routes a HEAD request to a Middleware when the request url matches the url pattern. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).


<h3 id='router-options'>Router::options</h3>

<h4 class='variant'>void options(const char* urlPattern, Middleware* middleware);</h4>

<h4 class='variant'>void options(Middleware* middleware);</h4>


Routes a OPTIONS request to a Middleware when the request url matches the url pattern. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).


<h3 id='router-patch'>Router::patch</h3>

<h4 class='variant'>void patch(const char* urlPattern, Middleware* middleware);</h4>

<h4 class='variant'>void patch(Middleware* middleware);</h4>


Routes a PATCH request to a Middleware when the request url matches the url pattern. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).


<h3 id='router-post'>Router::post</h3>

<h4 class='variant'>void post(const char* urlPattern, Middleware* middleware);</h4>

<h4 class='variant'>void post(Middleware* middleware);</h4>


Routes a POST request to a Middleware when the request url matches the url pattern. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).


<h3 id='router-put'>Router::put</h3>

<h4 class='variant'>void put(const char* urlPattern, Middleware* middleware);</h4>

<h4 class='variant'>void put(Middleware* middleware);</h4>


Routes a PUT request to a Middleware when the request url matches the url pattern. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).


<h3 id='router-use'>Router::use</h3>

<h4 class='variant'>void use(const char* path, Middleware* middleware);</h4>

<h4 class='variant'>void use(Router::Middleware* middleware);</h4>

<h4 class='variant'>void use(const char* path, Router* router);</h4>

<h4 class='variant'>void use(Router * router);</h4>


Makes the Middleware executed when the request URL beginning matches the prefix of the router. The overloaded version is used for nesting routers.

If the path parameter is not specified it's set to NULL the middleware or router will be executed on all requests.

For more information, see the [routing guide](/guide/routing.html).
