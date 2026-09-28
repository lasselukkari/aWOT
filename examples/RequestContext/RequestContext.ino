#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <aWOT.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
  
WiFiServer server(80);
App app;

struct context {
  IPAddress ip;
  unsigned long start;
  Request::MethodType method;
  char path[100];
  int status;
};

void fillContext(Request &req, Response &res) {
  context* ctx = (context*)req.context;
  ctx->method = req.method();
  strlcpy(ctx->path, req.path(), sizeof(ctx->path));
  req.next();
}

void setStatus(Request &req, Response &res) {
  context* ctx = (context*)req.context;
  ctx->status = res.statusSent();
}

void indexHandler(Request &req, Response &res) {
  context* ctx = (context*)req.context;

  res.print("Reading request took ");
  res.print(millis() - ctx->start);
  res.println(" ms");
  res.print("Your IP is: ");
  res.println(ctx->ip);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.use(&fillContext);
  app.get("/", &indexHandler);
  app.finally(&setStatus);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    context ctx = {};
    ctx.ip = client.remoteIP();
    ctx.start = millis();
    App::ProcessOptions options;
    options.context = &ctx;
    app.process(&client, options);
    client.stop();

    switch (ctx.method) {
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
          Serial.print("PATCH ");
          break;
        }
      case  Request::DELETE: {
          Serial.print("DELETE ");
          break;
        }
      case  Request::UNKNOWN: {
          Serial.print("UNKNOWN ");
          break;
        }
      default: {}
    }

    Serial.print(ctx.path);
    Serial.print(" ");
    Serial.print(millis() - ctx.start);
    Serial.print("ms ");
    Serial.println(ctx.status);
  }
}
