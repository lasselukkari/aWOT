---
layout: page
title: Writing middleware for use in aWOT apps
menu: guide
lang: en
redirect_from: "/guide/middleware.html"
---
# Middleware

_Middleware_ functions are functions that have access to the [request](/{{ page.lang }}/3x/api.html#req)  (`req`) and the [response](/{{ page.lang }}/3x/api.html#res) (`res`) parameters.

Middleware functions can execute any code and optionally end the request-response cycle.

If the current middleware function wants end the request-response cycle, it must call `res.end()` to prevent any further middleware from being executed.
<h2>Example</h2>

Here is an example of a simple "Hello World" aWOT application.
In this article will define and add a middleware function to the application called `myLogger` that prints a simple log message.

```arduino
#include <WiFi.h.h>
#include <aWOT.h>
  
WiFiServer server(80);
Application app;
  
void index(Request &req, Response &res) {
  res.print("Hello World!");
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

<h3>Middleware function myLogger</h3>
Here is a simple example of a middleware function called "myLogger". This function just prints "LOGGED" when a request to the app passes through it.

```arduino
void myLogger(Request &req, Response &res) {
  Serial.println("LOGGED");
}
```

To load the middleware function, call `app.use()`, specifying the middleware function.
For example, the following code loads the `myLogger` middleware function before the route to the root path (/).

```arduino
#include <WiFi.h>
#include <aWOT.h>
  
WiFiServer server(80);
Application app;

void myLogger(Request &req, Response &res) {
  Serial.println("LOGGED");
}
  
void index(Request &req, Response &res) {
  res.print("Hello World!");
  res.end();
}
  
void setup() {
  Serial.begin(115200);
  
  WiFi.begin("ssid", "password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.use(&myLogger);
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
The middleware function `myLogger` simply prints a message, then passes the control to the next middleware function in the chain. Every time the app receives a request, it prints the message "LOGGED" to the terminal.

The order of middleware loading is important: middleware functions that are loaded first are also executed first.

If `myLogger` is loaded after the route to the root path, the request never reaches it and the app doesn't print "LOGGED", because the route handler of the root path terminates the request-response cycle by calling res.end().

