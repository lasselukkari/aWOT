#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request & req, Response & res) {
  char name[6];
  char value[6];

  while (req.left()) {
    if(!req.form(name, 6, value, 6)) {
      return res.sendStatus(400);
    }

    res.print(name);
    res.print(": ");
    res.println(value);
  }
}

unittest(form_params) {
  const char *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 23" CRLF
    CRLF
    "test1=test1&test2=test2";

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "test1: test1" CRLF
    "test2: test2" CRLF;
    
  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(form_params_name_too_long) {
  const char *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 23" CRLF
    CRLF
    "test1*=test1&test2=test2";

  const char *expected =
    "HTTP/1.1 400 Bad Request" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "Bad Request";
    
  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(form_params_value_too_long) {
  const char *request =
    "POST / HTTP/1.0" CRLF
    "Content-Length: 23" CRLF
    CRLF
    "test1=test1*&test2=test2";

  const char *expected =
    "HTTP/1.1 400 Bad Request" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "Bad Request";
    
  MockStream stream(request);
  Application app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
