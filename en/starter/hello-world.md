---
layout: page
title: aWOT "Hello World" example
menu: starter
lang: en
redirect_from: "/starter/hello-world.html"
---

# Hello world example

Embedded below is essentially the simplest aWOT app you can create.

<div id="hello-example"><pre><code class="language-js">#include &lt;WiFi.h&gt;
#include &lt;aWOT.h&gt;
  
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
    client.flush();
  }
}
</code></pre></div>

This app starts a server and listens on port 80 for connections. The app responds with “Hello World!” for requests
to the root URL (<code>/</code>) or <em>route</em>. For every other path, it will respond with a <strong>404 Not Found</strong>.

The example above is actually a working server: Go ahead and upload it to your Arduino. Then, load `http://your.arduino.ip/` in a browser to see the output.
