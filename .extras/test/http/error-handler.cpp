#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/SharedClient.h"

struct ErrorObservation {
  int code = 0;
  int errorCalls = 0;
  int routeCalls = 0;
  int finalCalls = 0;
};

static void recordError(Request& req, int code) {
  ErrorObservation& observation = *static_cast<ErrorObservation*>(req.context);
  observation.code = code;
  ++observation.errorCalls;
}

static void customErrorHandler(Request& req, Response& res, int code) {
  recordError(req, code);
  res.status(code);
  res.print("Custom error: ");
  res.print(code);
  res.end();
}

static void jsonErrorHandler(Request& req, Response& res, int code) {
  recordError(req, code);
  res.status(code);
  res.set("Content-Type", "application/json");
  res.print("{\"error\":");
  res.print(code);
  res.print("}");
  // Finish the custom response so the default error body is not appended.
  res.end();
}

static void emptyErrorHandler(Request& req, Response& res, int code) {
  recordError(req, code);
  res.status(code);
  res.set("Content-Length", "0");
  res.end();
}

static void recordRoute(Request& req, Response& res) {
  ++static_cast<ErrorObservation*>(req.context)->routeCalls;
  res.print("ready");
}

static void recordFinal(Request& req, Response&) {
  ++static_cast<ErrorObservation*>(req.context)->finalCalls;
}

unittest(error_handler_not_found) {
  const char* request = "GET /nonexistent HTTP/1.0" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 404 Not Found" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Custom error: 404";

  SharedClient client;
  client.feed(request);
  App app;
  ErrorObservation observation;
  App::ProcessOptions options;
  options.context = &observation;

  app.onError(customErrorHandler);
  App::ProcessResult result = app.process(&client, options);

  assertEqual(404, observation.code);
  assertEqual(1, observation.errorCalls);
  assertEqual(std::string(expected), client.outputString());
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
}

unittest(malformed_url_uses_custom_json_error_response) {
  const char* request = "GET /bad%GG HTTP/1.1" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 400 Bad Request" CRLF "Content-Type: application/json" CRLF
      "Connection: close" CRLF CRLF "{\"error\":400}";

  SharedClient client;
  client.feed(request);
  App app;
  app.get(recordRoute);
  app.onError(jsonErrorHandler);
  ErrorObservation observation;
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertEqual(std::string(expected), client.outputString());
  assertEqual(400, observation.code);
  assertEqual(1, observation.errorCalls);
  assertEqual(0, observation.routeCalls);
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
  assertFalse(client.connected());
}

unittest(bodyless_custom_parser_error_completes_headers) {
  const char* request = "BOGUS / HTTP/1.1" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 400 Bad Request" CRLF "Content-Length: 0" CRLF
      "Content-Type: text/plain" CRLF "Connection: close" CRLF CRLF;

  SharedClient client;
  client.feed(request);
  App app;
  app.use(recordRoute);
  app.onError(emptyErrorHandler);
  ErrorObservation observation;
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertEqual(400, observation.code);
  assertEqual(1, observation.errorCalls);
  assertEqual(0, observation.routeCalls);
  assertEqual(std::string(expected), client.outputString());
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
  assertFalse(client.connected());
}

unittest(url_exceeding_supplied_buffer_returns_uri_too_long) {
  const char* request = "GET /status/temperature HTTP/1.1" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 414 URI Too Long" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "URI Too Long";

  SharedClient client;
  client.feed(request);
  App app;
  app.get(recordRoute);
  ErrorObservation observation;
  // The supplied buffer sets the URL limit, independent of the board default.
  char urlBuffer[8];
  App::ProcessOptions options;
  options.urlBuffer = urlBuffer;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertEqual(std::string(expected), client.outputString());
  assertEqual(0, observation.routeCalls);
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
  assertFalse(client.connected());
}

unittest(unsupported_http_version_returns_version_not_supported) {
  const char* request = "GET /status HTTP/2.0" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 505 HTTP Version Not Supported" CRLF
      "Content-Type: text/plain" CRLF "Connection: close" CRLF CRLF
      "HTTP Version Not Supported";

  SharedClient client;
  client.feed(request);
  App app;
  app.get("/status", recordRoute);
  ErrorObservation observation;
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertEqual(std::string(expected), client.outputString());
  assertEqual(0, observation.routeCalls);
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
  assertFalse(client.connected());
}

unittest(incomplete_headers_use_custom_timeout_response_and_run_finally) {
  const char* request = "GET /status HTTP/1.1" CRLF "Host: local";
  const char* expected =
      "HTTP/1.1 408 Request Timeout" CRLF "Content-Type: application/json" CRLF
      "Connection: close" CRLF CRLF "{\"error\":408}";

  SharedClient client;
  client.feed(request);
  App app;
  // Ready bytes are read normally; missing header bytes time out immediately.
  app.setTimeout(0);
  app.get("/status", recordRoute);
  app.onError(jsonErrorHandler);
  app.finally(recordFinal);
  ErrorObservation observation;
  App::ProcessOptions options;
  options.context = &observation;

  App::ProcessResult result = app.process(&client, options);

  assertEqual(std::string(expected), client.outputString());
  assertEqual(408, observation.code);
  assertEqual(1, observation.errorCalls);
  assertEqual(0, observation.routeCalls);
  assertEqual(1, observation.finalCalls);
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
  assertFalse(client.connected());
}

unittest_main()
