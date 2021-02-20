<h2 id="app">Application</h2>
The Application class is used for mounting Middleware to Routers and to process the incoming HTTP requests.

##### Example

```arduino
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

<section markdown="1">
  {% include api/en/3x/app-del.md %}
</section>

<section markdown="1">
  {% include api/en/3x/app-finally.md %}
</section>

<section markdown="1">
  {% include api/en/3x/app-get.md %}
</section>

<section markdown="1">
  {% include api/en/3x/app-head.md %}
</section>

<section markdown="1">
  {% include api/en/3x/app-header.md %}
</section>

<section markdown="1">
  {% include api/en/3x/app-options.md %}
</section>

<section markdown="1">
  {% include api/en/3x/app-patch.md %}
</section>

<section markdown="1">
  {% include api/en/3x/app-post.md %}
</section>

<section markdown="1">
  {% include api/en/3x/app-process.md %}
</section>

<section markdown="1">
  {% include api/en/3x/app-put.md %}
</section>

<section markdown="1">
  {% include api/en/3x/app-use.md %}
</section>
