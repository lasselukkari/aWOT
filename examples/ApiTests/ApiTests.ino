#include <WiFi.h>
#include "aWOT.h"

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

WiFiServer server(80);
Application app;
Router router("/router");
Router useRouter("/routeruse");

void cors(Request &req, Response &res) {
  res.set("Access-Control-Allow-Origin", "*");
  res.set("Access-Control-Allow-Methods", "GET, HEAD, POST, PUT, PATCH, DELETE, OPTIONS");
  res.set("Access-Control-Allow-Headers", "Content-Type, Test1, Test2");

  if (req.method() == Request::OPTIONS) {
    res.status(204);
    res.end();
  }
}

void index(Request &req, Response &res) {
  while (req.left() && !req.timeout()) {
    res.write(req.read());
  }
  res.set("Content-Type", "text/plain");
  res.print("/");
}

void use(Request &req, Response &res) {
  res.status(204);
  res.end();
}

void noContent(Request &req, Response &res) {
  res.status(204);
}

void queryParams(Request &req, Response &res) {
  char test1[10];
  char test2[10];

  if (!req.query("test1", test1, 10)) {
    return res.sendStatus(400);
  }

  if (!req.query("test2", test2, 10)) {
    return res.sendStatus(400);
  }

  res.set("Content-Type", "text/plain");
  res.print(test1);
  res.print(" ");
  res.print(test2);
}

void routeNameParams(Request &req, Response &res) {
  char test1[10];
  char test2[10];
  if (!req.route("test1", test1, 10)) {
    return res.sendStatus(400);
  }

  if (!req.route("test2", test2, 10)) {
    return res.sendStatus(400);
  }

  res.set("Content-Type", "text/plain");
  res.print(test1);
  res.print(" ");
  res.print(test2);
}

void routeNumberParams(Request &req, Response &res) {
  char test1[10];
  char test2[10];
  if (!req.route(2, test1, 10)) {
    return res.sendStatus(400);
  }

  if (!req.route(4, test2, 10)) {
    return res.sendStatus(400);
  }

  res.set("Content-Type", "text/plain");
  res.print(test1);
  res.print(" ");
  res.print(test2);
}


void forms(Request &req, Response &res) {
  char name[10];
  char value[10];

  res.set("Content-Type", "text/plain");
  while (req.left()) {
    if (!req.form(name, 10, value, 10)) {
      return res.sendStatus(400);
    }

    res.print(name);
    res.print(":");
    res.print(value);
    if (req.left()) {
      res.print(" ");
    }
  }
}

char test1Header[10];
char test2Header[10];
void headers(Request &req, Response &res) {
  char * test1 = req.get("Test1");
  char * test2 = req.get("Test2");

  res.set("Content-Type", "text/plain");
  res.print(test1);
  res.print(" ");
  res.print(test2);
}

void urlEscaping(Request &req, Response &res) {
  res.set("Content-Type", "text/plain");
  res.print(req.path());
  res.print(" ");
  res.print(req.query());
}

void fail(Request &req, Response &res) {
  res.sendStatus(400);
}

void unauthorized(Request &req, Response &res) {
  res.sendStatus(401);
}

void forbidden(Request &req, Response &res) {
  res.sendStatus(403);
}

void notFound(Request &req, Response &res) {
  res.sendStatus(404);
}

void serverError(Request &req, Response &res) {
  res.sendStatus(500);
}

byte readWriteBuffer[1001];
void readWrite(Request &req, Response &res) {
  if (!req.body(readWriteBuffer, 1001)) {
    return res.sendStatus(400);
  }
  res.set("Content-Type", "application/binary");
  res.write(readWriteBuffer, 1000);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.use(&cors);
  app.get("/", &index);

  app.get("/get", &noContent);
  app.post("/post", &noContent);
  app.put("/put", &noContent);
  app.patch("/patch", &noContent);
  app.del("/delete", &noContent);
  app.all("/all", &noContent);

  app.get("/query", &queryParams);
  app.get("/route/name/:test1/params/:test2", &routeNameParams);
  app.get("/route/number/test1/params/test2", &routeNumberParams);
  app.post("/postparams", &forms);

  router.get("/get", &noContent);
  router.get("/query", &queryParams);
  router.get("/route/name/:test1/params/:test2", &routeNameParams);
  router.get("/route/number/test1/params/test2", &routeNumberParams);
  app.route(&router);

  app.header("Test1", test1Header, 10);
  app.header("Test2", test2Header, 10);
  app.get("/headers", &headers);

  app.get("/url escaping", &urlEscaping);

  app.get("/fail", &fail);
  app.get("/unauthorized", &unauthorized);
  app.get("/forbidden", &forbidden);
  app.get("/notfound", &notFound);
  app.get("/servererror", &serverError);

  useRouter.use(&use);
  app.route(&useRouter);

  app.post("/readwrite", &readWrite);

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }
}
