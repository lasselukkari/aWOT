#include <ArduinoUnitTests.h>

#include "../../src/aWOT.h"
#include "./mocks/MockStream.h"

int capturedErrorCode = 0;

void customErrorHandler(Request& req, Response& res, int code) {
  capturedErrorCode = code;
  res.status(code);
  res.print("Custom error: ");
  res.print(code);
  res.end();
}

unittest(error_handler_not_found) {
  capturedErrorCode = 0;

  const char* request = "GET /nonexistent HTTP/1.0" CRLF CRLF;

  MockStream stream(request);
  App app;

  app.onError(&customErrorHandler);
  app.process(&stream);

  assertEqual(404, capturedErrorCode);
  String response = stream.response();
  assertTrue(response.indexOf("Custom error: 404") >= 0);
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

struct EarlyErrorObservation {
  int code;
  bool errorPathEmpty;
  bool finalCalled;
  bool finalPathEmpty;
};

void observeEarlyError(Request& req, Response& res, int code) {
  EarlyErrorObservation& observation =
      *static_cast<EarlyErrorObservation*>(req.context);
  observation.code = code;
  observation.errorPathEmpty = req.path()[0] == '\0';
  res.sendStatus(code);
}

void observeEarlyFinal(Request& req, Response&) {
  EarlyErrorObservation& observation =
      *static_cast<EarlyErrorObservation*>(req.context);
  observation.finalCalled = true;
  observation.finalPathEmpty = req.path()[0] == '\0';
}

unittest(early_parser_errors_have_an_empty_path_in_callbacks) {
  const char* requests[] = {"BOGUS / HTTP/1.1" CRLF CRLF, ""};
  const int codes[] = {400, 408};
  for (int i = 0; i < 2; ++i) {
    App app;
    app.setTimeout(0);
    app.onError(observeEarlyError);
    app.finally(observeEarlyFinal);
    char url[8];
    memset(url, 'X', sizeof(url));
    EarlyErrorObservation observation = {0, false, false, false};
    App::ProcessOptions options;
    options.urlBuffer = url;
    options.context = &observation;
    MockStream stream(requests[i]);
    app.process(&stream, options);

    assertEqual(codes[i], observation.code);
    assertTrue(observation.errorPathEmpty);
    assertTrue(observation.finalCalled);
    assertTrue(observation.finalPathEmpty);
  }
}

unittest_main()
