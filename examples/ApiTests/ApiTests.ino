#include <WiFi.h>
#include "aWOT.h"

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

WiFiServer server(80);
WebApp app;
Router router("/router");
Router useRouter("/routeruse");

void cors(Request &req, Response &res) {
  res.set("Access-Control-Allow-Origin", "*");
  res.set("Access-Control-Allow-Methods", "GET, HEAD, POST, PUT, PATCH, DELETE, OPTIONS");
  res.set("Access-Control-Allow-Headers", "Content-Type, Test1, Test2");

  if (req.method() == Request::OPTIONS) {
    res.noContent();
    res.end();
  }
}

void index(Request &req, Response &res) {
  res.success("text/plain");
  res.print("/");
}

void use(Request &req, Response &res) {
  res.noContent();
  res.end();
}

void noContent(Request &req, Response &res) {
  res.noContent();
}

void queryParams(Request &req, Response &res) {
  char test1[64];
  char test2[64];

  req.query("test1", test1, 64);
  req.query("test2", test2, 64);

  res.success("text/plain");
  res.print(test1);
  res.print(" ");
  res.print(test2);
}

void routeNameParams(Request &req, Response &res) {
  char test1[64];
  char test2[64];
  req.route("test1", test1, 64);
  req.route("test2", test2, 64);

  res.success("text/plain");
  res.print(test1);
  res.print(" ");
  res.print(test2);
}

void routeNumberParams(Request &req, Response &res) {
  char test1[64];
  char test2[64];
  req.route(2, test1, 64);
  req.route(4, test2, 64);

  res.success("text/plain");
  res.print(test1);
  res.print(" ");
  res.print(test2);
}


void postParams(Request &req, Response &res) {
  char name[10];
  char value[64];

  res.success("text/plain");
  while (req.contentLeft()) {
    req.postParam(name, 10, value, 64);
    res.print(name);
    res.print(":");
    res.print(value);
    if (req.contentLeft()) {
      res.print(" ");
    }
  }
}

char test1Header[200];
char test2Header[200];
void headers(Request &req, Response &res) {
  char * test1 = req.header("Test1");
  char * test2 = req.header("Test2");

  res.success("text/plain");
  res.print(test1);
  res.print(" ");
  res.print(test2);
}

void urlEscaping(Request &req, Response &res) {
  res.success("text/plain");
  res.print(req.urlPath());
  res.print(" ");
  res.print(req.query());
}

void fail(Request &req, Response &res) {
  res.fail();
}

void unauthorized(Request &req, Response &res) {
  res.unauthorized();
}

void forbidden(Request &req, Response &res) {
  res.forbidden();
}

void notFound(Request &req, Response &res) {
  res.notFound();
}

void serverError(Request &req, Response &res) {
  res.serverError();
}

uint8_t readWriteBuffer[10000];
void readWrite(Request &req, Response &res) {

  int i = 0;
  while (req.contentLeft() && i < 10000) {
    readWriteBuffer[i++] = req.read();
  }
  res.success("application/binary");
  res.write(readWriteBuffer, i);
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
  app.post("/postparams", &postParams);

  router.get("/get", &noContent);
  router.get("/query", &queryParams);
  router.get("/route/name/:test1/params/:test2", &routeNameParams);
  router.get("/route/number/test1/params/test2", &routeNumberParams);
  app.use(&router);

  app.readHeader("Test1", test1Header, 200);
  app.readHeader("Test2", test2Header, 200);
  app.get("/headers", &headers);

  app.get("/url escaping", &urlEscaping);

  app.get("/fail", &fail);
  app.get("/unauthorized", &unauthorized);
  app.get("/forbidden", &forbidden);
  app.get("/notfound", &notFound);
  app.get("/servererror", &serverError);

  useRouter.use(&use);
  app.use(&useRouter);

  app.post("/readwrite", &readWrite);

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }
}
