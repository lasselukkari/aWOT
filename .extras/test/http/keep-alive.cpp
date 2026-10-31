#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/SharedClient.h"

static void allowKeepAlive(Request& req, Response& res) {
  res.set("Connection", "keep-alive");
  req.next();
}

static void greet(Request& req, Response& res) {
  char field[8];
  char name[16];
  if (!req.form(field, sizeof(field), name, sizeof(name)) ||
      strcmp(field, "name") != 0 || req.left() != 0) {
    return res.sendStatus(400);
  }

  res.print("Hello, ");
  res.print(name);
  res.print("!");
}

static void readStatus(Request&, Response& res) { res.print("ready"); }

unittest(reuse_connection_after_chunked_post_then_close_after_get) {
  const char* request =
      "POST /greeting HTTP/1.1" CRLF
      "Content-Type: application/x-www-form-urlencoded" CRLF
      "Transfer-Encoding: chunked" CRLF CRLF "7" CRLF "name=Ad" CRLF "1" CRLF
      "a" CRLF "0" CRLF CRLF "GET /status HTTP/1.1" CRLF
      "Connection: close" CRLF CRLF;

  const char* expected =
      "HTTP/1.1 200 OK" CRLF "Connection: keep-alive" CRLF
      "Content-Type: text/plain" CRLF "Transfer-Encoding: chunked" CRLF CRLF
      "b" CRLF "Hello, Ada!" CRLF "0" CRLF CRLF "HTTP/1.1 200 OK" CRLF
      "Content-Type: text/plain" CRLF "Connection: close" CRLF CRLF "ready";

  SharedClient client;
  client.feed(request);
  App app;
  app.setTimeout(0);
  app.use(allowKeepAlive);
  app.post("/greeting", greet);
  app.get("/status", readStatus);

  App::ProcessResult first = app.process(&client);
  assertTrue(first.keepAlive);
  assertFalse(first.responseOpen);
  assertTrue(client.connected());

  // The server loop reuses the client only when process() allows it.
  App::ProcessResult second;
  if (first.keepAlive) second = app.process(&client);
  if (!second.keepAlive) client.stop();

  assertEqual(std::string(expected), client.outputString());
  assertFalse(second.keepAlive);
  assertFalse(second.responseOpen);
  assertFalse(client.connected());
  assertEqual(0, client.available());
}

unittest_main()
