#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request &req, Response &res) {
  char test1[10];
  char test2[10];
  if (!req.route(1, test1, 10)) {
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

unittest(route_params_ok) {
  char const *request = 
  "GET /route/test1/params/test2 HTTP/1.0" CRLF 
  CRLF;

  char const *expected = 
  "HTTP/1.1 200 OK" CRLF 
  "Content-Type: text/plain" CRLF
  "Connection: close" CRLF 
  CRLF 
  "test1 test2";

  MockStream stream(request);
  Application app;

  app.get("/route/:test1/params/:test2", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(route_params_value_too_long) {
  char const *request = 
  "GET /route/morethantenchars/params/test2 HTTP/1.0" CRLF 
  CRLF;

  char const *expected = 
  "HTTP/1.1 400 Bad Request" CRLF
  "Content-Type: text/plain" CRLF
  "Connection: close" CRLF
  CRLF
  "Bad Request";;

  MockStream stream(request);
  Application app;

  app.get("/route/:test1/params/:test2", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(router_route_params) {
  char const *request = 
   "GET /router/route/test1/params/test2 HTTP/1.0" CRLF
  CRLF;

  char const *expected = 
  "HTTP/1.1 200 OK" CRLF 
  "Content-Type: text/plain" CRLF
  "Connection: close" CRLF 
  CRLF 
  "test1 test2";

  MockStream stream(request);
  Router router("/router");
  Application app;

  router.get("/route/:test1/params/:test2", &handler);
  app.route(&router);

  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
