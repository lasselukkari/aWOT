#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include <string>
#include <vector>

#include "mocks/MockStream.h"

#ifdef STD_FUNCTION_MIDDLEWARE
struct StatefulAppCallback {
  StatefulAppCallback(App* app, MockStream* nested, std::vector<int>* calls)
      : app(app), nested(nested), calls(calls), count(0) {}

  void operator()(Request&, Response&) {
    calls->push_back(++count);
    if (count == 1) app->process(nested);
  }

  App* app;
  MockStream* nested;
  std::vector<int>* calls;
  int count;
};

unittest(finally_callback_can_clear_itself_and_use_captures) {
  const std::string request = "GET / HTTP/1.1" CRLF CRLF;
  const std::string captured(256, 'f');
  std::string observed;
  MockStream first(request.c_str());
  MockStream nested(request.c_str());
  App app;
  int calls = 0;
  app.finally(
      [&app, &nested, &observed, &calls, captured](Request&, Response&) {
        ++calls;
        app.finally(nullptr);
        app.process(&nested);
        observed = captured;
      });

  app.process(&first);

  assertEqual(1, calls);
  assertEqual(captured, observed);
}

unittest(not_found_callback_can_clear_itself_and_use_captures) {
  const std::string request = "GET /missing HTTP/1.1" CRLF CRLF;
  const std::string captured(256, 'n');
  std::string observed;
  MockStream first(request.c_str());
  MockStream nested(request.c_str());
  App app;
  int calls = 0;
  app.notFound(
      [&app, &nested, &observed, &calls, captured](Request&, Response&) {
        ++calls;
        app.notFound(nullptr);
        app.process(&nested);
        observed = captured;
      });

  app.process(&first);

  assertEqual(1, calls);
  assertEqual(captured, observed);
}

unittest(app_callbacks_preserve_mutable_state_during_nested_requests) {
  const char* request = "GET / HTTP/1.1" CRLF CRLF;
  {
    MockStream first(request), nested(request), later(request);
    App app;
    std::vector<int> calls;
    app.finally(StatefulAppCallback(&app, &nested, &calls));

    app.process(&first);
    app.process(&later);

    assertTrue(calls == std::vector<int>({1, 2, 3}));
  }
  {
    MockStream first(request), nested(request), later(request);
    App app;
    std::vector<int> calls;
    app.notFound(StatefulAppCallback(&app, &nested, &calls));

    app.process(&first);
    app.process(&later);

    assertTrue(calls == std::vector<int>({1, 2, 3}));
  }
}

unittest(callback_replacement_survives_nested_and_later_requests) {
  const char* request = "GET / HTTP/1.1" CRLF CRLF;
  MockStream first(request), nested(request), later(request);
  App app;
  int originalCalls = 0;
  int replacementCalls = 0;
  const std::string captured(256, 'r');
  std::string observed;
  app.finally([&](Request&, Response&) {
    ++originalCalls;
    app.finally([&](Request&, Response&) { ++replacementCalls; });
    app.process(&nested);
    observed = captured;
  });

  app.process(&first);
  app.process(&later);

  assertEqual(1, originalCalls);
  assertEqual(2, replacementCalls);
  assertEqual(captured, observed);
}
#endif

unittest_main()
