#include <ArduinoUnitTests.h>

#include <functional>

#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

class MyController {
 public:
  MyController() : visited(false) {}

  void handle(Request& req, Response& res) {
    visited = true;
    res.set("Content-Type", "text/plain");
    res.print("member function");
  }

  bool visited;
};

unittest(member_function_route) {
  const char* request = "GET / HTTP/1.0" CRLF CRLF;
  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "member function";

  MockStream stream(request);
  Application app;
  MyController controller;

  app.get("/", &controller, &MyController::handle);
  app.process(&stream);

  assertEqual(expected, stream.response());
  assertTrue(controller.visited);
}

unittest_main()
