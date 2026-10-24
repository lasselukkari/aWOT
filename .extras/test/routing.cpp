#include <ArduinoUnitTests.h>

#include "../../src/aWOT.h"
#include "./mocks/MockStream.h"

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
  App app;

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

void testHandler(Request& req, Response& res) { res.print("test"); }

unittest(use_route_with_path) {
  const char* request = "POST /usepath HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  App app;

  app.use("/usepath", &testHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(use_route_without_path) {
  const char* request = "POST /usepath HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  App app;

  app.use(&testHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(get_route_without_path) {
  const char* request = "GET /somepath HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  App app;

  app.get(&testHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(router_with_path) {
  const char* request = "GET /router/route HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  App app;
  Router router;

  router.get("/route", &testHandler);
  app.use("/router", &router);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(router_without_path) {
  const char* request = "GET /route HTTP/1.0" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "test";

  MockStream stream(request);
  App app;
  Router router;

  router.get("/route", &testHandler);
  app.use(&router);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(router_mount_slashes) {
  const char* mounts[] = {"/", "/api", "/api/"};
  const char* requests[] = {
      "GET /route HTTP/1.0" CRLF CRLF,
      "GET /api/route HTTP/1.0" CRLF CRLF,
      "GET /api/route HTTP/1.0" CRLF CRLF,
  };
  for (int i = 0; i < 3; ++i) {
    MockStream stream(requests[i]);
    App app;
    Router router;
    router.get("/route", testHandler);
    app.use(mounts[i], &router);
    app.process(&stream);
    assertTrue(strstr(stream.response(), "HTTP/1.1 200 ") == stream.response());
  }
}

unittest(router_mount_root_with_optional_slash) {
  const char* mounts[] = {"/api", "/api/"};
  const char* requests[] = {"GET /api HTTP/1.0" CRLF CRLF,
                            "GET /api/ HTTP/1.0" CRLF CRLF};
  for (const char* mount : mounts) {
    for (const char* request : requests) {
      MockStream stream(request);
      App app;
      Router router;
      router.get("/", testHandler);
      app.use(mount, &router);
      app.process(&stream);
      assertTrue(strstr(stream.response(), "HTTP/1.1 200 ") ==
                 stream.response());
    }
  }
}

unittest(router_mount_requires_a_path_segment_boundary) {
  const char* mounts[] = {"/api", "/api/"};
  for (const char* mount : mounts) {
    MockStream stream("GET /api2/route HTTP/1.0" CRLF CRLF);
    App app;
    Router router;
    router.get("2/route", testHandler);
    router.get("/route", testHandler);
    app.use(mount, &router);
    app.process(&stream);
    assertTrue(strstr(stream.response(), "HTTP/1.1 404 ") == stream.response());
  }
}

unittest(nested_slash_mounts_preserve_named_parameters) {
  MockStream stream("GET /api/route/value HTTP/1.0" CRLF CRLF);
  App app;
  Router root;
  Router child;
  child.get("/route/:test1", handler);
  root.use("/api/", &child);
  app.use("/", &root);
  app.process(&stream);
  assertTrue(strstr(stream.response(), "HTTP/1.1 200 ") == stream.response());
  assertTrue(strstr(stream.response(), CRLF CRLF "value") != nullptr);
}

unittest_main()
