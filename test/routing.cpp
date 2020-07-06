#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void beforeAll(Request &req, Response &res) {
  res.print("beforeAll-");
}

void beforeRouter(Request &req, Response &res) {
  res.print("beforeRouter-");
}

void handler(Request &req, Response &res) {
  char test1[10];
  req.route("test1", test1, 10);
  res.print(test1);
}

void afterRouter(Request &req, Response &res) {
  res.print("-afterRouter");
}

void afterAll(Request &req, Response &res) {
  res.print("-afterAll");
}

unittest(nested_routers) {
  char const *request =
    "GET /router1/router2/router3/route/test HTTP/1.0" CRLF
    CRLF;

  char const *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "beforeAll-beforeRouter-test-afterRouter-afterAll";

  MockStream stream(request);
  Router router1;
  Router router2;
  Router router3;
  Application app;

  router3.use(&beforeRouter);
  router3.get("/route/:test1", &handler);
  router3.use(&afterRouter);
  router2.use("/router3", &router3);
  router1.use("/router2", &router2);

  app.use(&beforeAll);
  app.use("/router1", &router1);
  app.use(&afterAll);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
