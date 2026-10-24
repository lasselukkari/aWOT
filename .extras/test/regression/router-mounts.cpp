#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

void handler(Request& req, Response& res) {
  char test1[10];
  req.route("test1", test1, 10);
  res.print(test1);
  req.next();
}

void testHandler(Request& req, Response& res) { res.print("test"); }

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
