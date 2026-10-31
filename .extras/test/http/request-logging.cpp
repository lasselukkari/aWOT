#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

struct RequestLog {
  int calls;
  int status;
  int acceptedBytes;
  int resourceCalls;
  char path[32];
};

static void logRequest(Request& req, Response& res) {
  RequestLog& log = *static_cast<RequestLog*>(req.context);
  ++log.calls;
  log.status = res.statusSent();
  // Counts bytes accepted before finalization, not confirmed delivery or
  // chunk framing. These responses use fixed-length or close delimiting.
  log.acceptedBytes = res.bytesSent();
  // The request path is borrowed storage; copy it before process() returns.
  strncpy(log.path, req.path(), sizeof(log.path) - 1);
  log.path[sizeof(log.path) - 1] = '\0';
}

static void requireToken(Request& req, Response& res) {
  const char* authorization = req.get("Authorization");
  if (!authorization || strcmp(authorization, "Bearer test-token") != 0) {
    res.set("WWW-Authenticate", "Bearer realm=\"device\"");
    res.set("Content-Length", "12");
    return res.sendStatus(401);
  }

  req.next();
}

static void privateStatus(Request& req, Response& res) {
  RequestLog& log = *static_cast<RequestLog*>(req.context);
  ++log.resourceCalls;
  res.set("Content-Length", "5");
  res.print("ready");
}

unittest(log_successful_request_once) {
  const char* request =
      "GET /private HTTP/1.1" CRLF "Authorization: Bearer test-token" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 200 OK" CRLF "Content-Length: 5" CRLF
      "Content-Type: text/plain" CRLF "Connection: close" CRLF CRLF "ready";

  RequestLog log = {};
  char authorization[32];
  Request::Header headers[] = {{"Authorization", authorization}};
  App::ProcessOptions options;
  options.headers = headers;
  options.context = &log;
  MockStream stream(request);
  App app;
  app.use(requireToken);
  app.get("/private", privateStatus);
  app.finally(logRequest);
  app.process(&stream, options);

  assertEqual(expected, stream.response());
  assertEqual(1, log.calls);
  assertEqual(200, log.status);
  assertEqual(static_cast<int>(strlen(expected)), log.acceptedBytes);
  assertEqual("/private", log.path);
  assertEqual(1, log.resourceCalls);
}

unittest(log_rejected_request_once) {
  const char* request = "GET /private HTTP/1.1" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 401 Unauthorized" CRLF
      "WWW-Authenticate: Bearer realm=\"device\"" CRLF "Content-Length: 12" CRLF
      "Content-Type: text/plain" CRLF "Connection: close" CRLF CRLF
      "Unauthorized";

  RequestLog log = {};
  char authorization[32];
  Request::Header headers[] = {{"Authorization", authorization}};
  App::ProcessOptions options;
  options.headers = headers;
  options.context = &log;
  MockStream stream(request);
  App app;
  app.use(requireToken);
  app.get("/private", privateStatus);
  app.finally(logRequest);
  app.process(&stream, options);

  assertEqual(expected, stream.response());
  assertEqual(1, log.calls);
  assertEqual(401, log.status);
  assertEqual(static_cast<int>(strlen(expected)), log.acceptedBytes);
  assertEqual("/private", log.path);
  assertEqual(0, log.resourceCalls);
}

unittest(log_unmatched_request_once) {
  const char* request = "GET /missing HTTP/1.1" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 404 Not Found" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Not Found";

  RequestLog log = {};
  App::ProcessOptions options;
  options.context = &log;
  MockStream stream(request);
  App app;
  app.get("/private", privateStatus);
  app.finally(logRequest);
  app.process(&stream, options);

  assertEqual(expected, stream.response());
  assertEqual(1, log.calls);
  assertEqual(404, log.status);
  assertEqual(static_cast<int>(strlen(expected)), log.acceptedBytes);
  assertEqual("/missing", log.path);
  assertEqual(0, log.resourceCalls);
}

unittest_main()
