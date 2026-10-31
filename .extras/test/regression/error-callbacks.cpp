#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

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
