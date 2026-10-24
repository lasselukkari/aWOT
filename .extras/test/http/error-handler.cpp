#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

int capturedErrorCode = 0;

void customErrorHandler(Request& req, Response& res, int code) {
  capturedErrorCode = code;
  res.status(code);
  res.print("Custom error: ");
  res.print(code);
  res.end();
}

void emptyErrorHandler(Request&, Response& res, int code) {
  capturedErrorCode = code;
  res.status(code);
  res.set("Content-Length", "0");
  res.end();
}

unittest(error_handler_not_found) {
  capturedErrorCode = 0;

  const char* request = "GET /nonexistent HTTP/1.0" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 404 Not Found" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Custom error: 404";

  MockStream stream(request);
  App app;

  app.onError(&customErrorHandler);
  app.process(&stream);

  assertEqual(404, capturedErrorCode);
  assertEqual(expected, stream.response());
}

unittest(error_handler_custom_response) {
  capturedErrorCode = 0;

  const char* request = "GET /nonexistent HTTP/1.0" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 404 Not Found" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Custom error: 404";

  MockStream stream(request);
  App app;

  app.onError(&customErrorHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(bodyless_custom_parser_error_completes_headers) {
  capturedErrorCode = 0;
  const char* request = "BOGUS / HTTP/1.1" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 400 Bad Request" CRLF "Content-Length: 0" CRLF
      "Content-Type: text/plain" CRLF "Connection: close" CRLF CRLF;

  MockStream stream(request);
  App app;
  app.onError(emptyErrorHandler);
  app.process(&stream);

  assertEqual(400, capturedErrorCode);
  assertEqual(expected, stream.response());
}

unittest_main()
