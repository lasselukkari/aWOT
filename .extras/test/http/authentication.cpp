#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

static int protectedCalls;

static void requireToken(Request& req, Response& res) {
  const char* authorization = req.get("Authorization");
  if (!authorization || strcmp(authorization, "Bearer test-token") != 0) {
    res.set("WWW-Authenticate", "Bearer realm=\"device\"");
    return res.sendStatus(401);
  }

  req.next();
}

static void privateResource(Request&, Response& res) {
  ++protectedCalls;
  res.print("private data");
}

unittest(valid_token_continues_to_protected_handler) {
  const char* request =
      "GET /private HTTP/1.1" CRLF "Authorization: Bearer test-token" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "private data";

  char authorization[32];
  Request::Header headers[] = {{"Authorization", authorization}};
  MockStream stream(request);
  App app;
  protectedCalls = 0;

  app.use(requireToken);
  app.get("/private", privateResource);
  app.process(&stream, headers);

  assertEqual(expected, stream.response());
  assertEqual(1, protectedCalls);
}

unittest(valid_token_ignores_surrounding_optional_whitespace) {
  const char* request = "GET /private HTTP/1.1" CRLF
                        "Authorization: \tBearer test-token \t" CRLF CRLF;

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "private data";

  char authorization[sizeof("Bearer test-token")];
  Request::Header headers[] = {{"Authorization", authorization}};
  MockStream stream(request);
  App app;
  protectedCalls = 0;

  app.use(requireToken);
  app.get("/private", privateResource);
  app.process(&stream, headers);

  assertEqual(expected, stream.response());
  assertEqual(1, protectedCalls);
}

unittest(missing_token_stops_before_protected_handler) {
  const char* request = "GET /private HTTP/1.1" CRLF CRLF;

  const char* expected = "HTTP/1.1 401 Unauthorized" CRLF
                         "WWW-Authenticate: Bearer realm=\"device\"" CRLF
                         "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "Unauthorized";

  char authorization[32];
  Request::Header headers[] = {{"Authorization", authorization}};
  MockStream stream(request);
  App app;
  protectedCalls = 0;

  app.use(requireToken);
  app.get("/private", privateResource);
  app.process(&stream, headers);

  assertEqual(expected, stream.response());
  assertEqual(0, protectedCalls);
}

unittest(invalid_token_stops_before_protected_handler) {
  const char* request = "GET /private HTTP/1.1" CRLF
                        "Authorization: Bearer wrong-token" CRLF CRLF;

  const char* expected = "HTTP/1.1 401 Unauthorized" CRLF
                         "WWW-Authenticate: Bearer realm=\"device\"" CRLF
                         "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "Unauthorized";

  char authorization[32];
  Request::Header headers[] = {{"Authorization", authorization}};
  MockStream stream(request);
  App app;
  protectedCalls = 0;

  app.use(requireToken);
  app.get("/private", privateResource);
  app.process(&stream, headers);

  assertEqual(expected, stream.response());
  assertEqual(0, protectedCalls);
}

unittest_main()
