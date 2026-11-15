#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

void beforeAll(Request& req, Response& res) {
  res.print("beforeAll-");
  req.next();
}

void beforeRouter(Request& req, Response& res) {
  res.print("beforeRouter-");
  req.next();
}

void handler(Request& req, Response& res) {
  char test1[10];
  req.route("test1", test1, 10);
  res.print(test1);
  req.next();
}

void afterRouter(Request& req, Response& res) {
  res.print("-afterRouter");
  req.next();
}

void afterAll(Request& req, Response& res) { res.print("-afterAll"); }

unittest(nested_routers) {
  const char* request =
      "GET /router1/router2/router3/route/test HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF
                         "beforeAll-beforeRouter-test-afterRouter-afterAll";

  MockStream stream(request);
  Router router1;
  Router router2;
  Router router3;
  HttpServer http;

  router3.use(&beforeRouter);
  router3.get("/route/:test1", &handler);
  router3.use(&afterRouter);
  router2.use("/router3", &router3);
  router1.use("/router2", &router2);

  http.use(&beforeAll);
  http.use("/router1", &router1);
  http.use(&afterAll);
  http.process(&stream);

  assertEqual(expected, stream.response());
}

void testHandler(Request& req, Response& res) { res.print("test"); }

unittest(use_route_with_path) {
  const char* request = "POST /usepath HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  HttpServer http;

  http.use("/usepath", &testHandler);
  http.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(use_route_without_path) {
  const char* request = "POST /usepath HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  HttpServer http;

  http.use(&testHandler);
  http.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(get_route_without_path) {
  const char* request = "GET /somepath HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  HttpServer http;

  http.get(&testHandler);
  http.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(router_with_path) {
  const char* request = "GET /router/route HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  HttpServer http;
  Router router;

  router.get("/route", &testHandler);
  http.use("/router", &router);
  http.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(router_without_path) {
  const char* request = "GET /route HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  HttpServer http;
  Router router;

  router.get("/route", &testHandler);
  http.use(&router);
  http.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
