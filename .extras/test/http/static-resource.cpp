#include <ArduinoUnitTests.h>
#include <aWOT.h>
#include <stdio.h>

#include "mocks/MockStream.h"

static const unsigned char homePage[] PROGMEM =
    "<!doctype html><title>aWOT</title><h1>Hello!</h1>\n";
static const char homePageTag[] = "\"home-v1\"";

static void serveHomePage(Request& req, Response& res) {
  res.set("ETag", homePageTag);

  // This example accepts a single exact tag returned by an earlier GET.
  const char* cachedTag = req.get("If-None-Match");
  if (cachedTag && strcmp(cachedTag, homePageTag) == 0) {
    return res.sendStatus(304);
  }

  char contentLength[12];
  snprintf(contentLength, sizeof(contentLength), "%u",
           static_cast<unsigned int>(sizeof(homePage) - 1));
  res.set("Content-Type", "text/html; charset=utf-8");
  res.set("Content-Length", contentLength);

  // Register the same handler for HEAD: aWOT suppresses these body bytes.
  res.writeP(homePage, sizeof(homePage) - 1);
}

unittest(get_flash_backed_home_page) {
  const char* request = "GET / HTTP/1.1" CRLF "Host: device.local" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "ETag: \"home-v1\"" CRLF
                         "Content-Type: text/html; charset=utf-8" CRLF
                         "Content-Length: 50" CRLF "Connection: close" CRLF CRLF
                         "<!doctype html><title>aWOT</title><h1>Hello!</h1>\n";

  char ifNoneMatch[32];
  Request::Header headers[] = {{"If-None-Match", ifNoneMatch}};
  MockStream stream(request);
  App app;
  app.get("/", serveHomePage);
  app.head("/", serveHomePage);

  App::ProcessResult result = app.process(&stream, headers);

  assertEqual(expected, stream.response());
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
}

unittest(head_returns_resource_headers_without_body) {
  const char* request = "HEAD / HTTP/1.1" CRLF "Host: device.local" CRLF CRLF;

  const char* expected =
      "HTTP/1.1 200 OK" CRLF "ETag: \"home-v1\"" CRLF
      "Content-Type: text/html; charset=utf-8" CRLF "Content-Length: 50" CRLF
      "Connection: close" CRLF CRLF;

  char ifNoneMatch[32];
  Request::Header headers[] = {{"If-None-Match", ifNoneMatch}};
  MockStream stream(request);
  App app;
  app.get("/", serveHomePage);
  app.head("/", serveHomePage);

  App::ProcessResult result = app.process(&stream, headers);

  assertEqual(expected, stream.response());
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
}

unittest(matching_etag_returns_not_modified_without_body) {
  const char* request = "GET / HTTP/1.1" CRLF "Host: device.local" CRLF
                        "If-None-Match: \"home-v1\"" CRLF CRLF;

  const char* expected = "HTTP/1.1 304 Not Modified" CRLF
                         "ETag: \"home-v1\"" CRLF "Connection: close" CRLF CRLF;

  char ifNoneMatch[32];
  Request::Header headers[] = {{"If-None-Match", ifNoneMatch}};
  MockStream stream(request);
  App app;
  app.get("/", serveHomePage);
  app.head("/", serveHomePage);

  App::ProcessResult result = app.process(&stream, headers);

  assertEqual(expected, stream.response());
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
}

unittest(stale_etag_returns_current_resource_and_tag) {
  const char* request = "GET / HTTP/1.1" CRLF "Host: device.local" CRLF
                        "If-None-Match: \"home-v0\"" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "ETag: \"home-v1\"" CRLF
                         "Content-Type: text/html; charset=utf-8" CRLF
                         "Content-Length: 50" CRLF "Connection: close" CRLF CRLF
                         "<!doctype html><title>aWOT</title><h1>Hello!</h1>\n";

  char ifNoneMatch[32];
  Request::Header headers[] = {{"If-None-Match", ifNoneMatch}};
  MockStream stream(request);
  App app;
  app.get("/", serveHomePage);
  app.head("/", serveHomePage);

  App::ProcessResult result = app.process(&stream, headers);

  assertEqual(expected, stream.response());
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
}

unittest_main()
