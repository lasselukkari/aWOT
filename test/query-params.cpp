#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request &req, Response &res) {
  char test1[6];
  char test2[6];

  if (!req.query("test1", test1, 6)) {
    return res.sendStatus(400);
  }

  if (!req.query("test2", test2, 6)) {
    return res.sendStatus(400);
  }

  res.set("Content-Type", "text/plain");
  res.print(test1);
  res.print(" ");
  res.print(test2);
}

unittest(query_params_ok) {
  char const *request =
    "GET /query?test1=test1&test2=test2 HTTP/1.0" CRLF
    CRLF;

  char const *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "test1 test2";

  MockStream stream(request);
  Application app;

  app.get("/query", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(query_params_value_too_long) {
  char const *request =
    "GET /query?test1=123456&test2=test2 HTTP/1.0" CRLF
    CRLF;

  char const *expected =
    "HTTP/1.1 400 Bad Request" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "Bad Request";;

  MockStream stream(request);
  Application app;

  app.get("/query", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(router_query_params) {
  char const *request =
    "GET /router/query?test1=test1&test2=test2 HTTP/1.0" CRLF
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

  router.get("/query", &handler);
  app.route(&router);

  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
