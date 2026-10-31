#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>
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

static bool waitForFlag(const std::atomic<bool>& flag) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while (!flag.load()) {
    if (std::chrono::steady_clock::now() >= deadline) return false;
    std::this_thread::yield();
  }
  return true;
}

struct ConcurrentCallbackState {
  std::atomic<bool> firstEntered{false};
  std::atomic<bool> secondEntered{false};
  std::atomic<bool> firstReturned{false};
  std::atomic<bool> secondReturned{false};
  std::atomic<bool> timedOut{false};
  int observed = 0;
};

// Keep this functor small enough for std::function's inline storage: the
// original bug moved that storage onto the first request's stack.
struct ConcurrentAppCallback {
  ConcurrentCallbackState* state;
  int marker;

  void operator()(Request& req, Response&) {
    if (!req.context) {
      state->firstEntered = true;
      if (!waitForFlag(state->secondEntered)) state->timedOut = true;
    } else {
      state->secondEntered = true;
      if (!waitForFlag(state->firstReturned)) state->timedOut = true;
      // The first process() call has returned, but this invocation must still
      // own the functor and be able to read its captures.
      state->observed = marker;
    }
  }
};

typedef void (App::*AppCallbackSetter)(Router::Middleware);

static bool overlappingRequestsRetainCallback(AppCallbackSetter setter) {
  const char* request = "GET / HTTP/1.1" CRLF CRLF;
  ConcurrentCallbackState state;
  App app;
  (app.*setter)(ConcurrentAppCallback{&state, 42});
  std::thread first([&]() {
    MockStream stream(request);
    app.process(&stream);
    state.firstReturned = true;
    if (!waitForFlag(state.secondReturned)) state.timedOut = true;
  });

  bool started = waitForFlag(state.firstEntered);
  MockStream second(request);
  App::ProcessOptions options;
  options.context = &state;
  app.process(&second, options);
  state.secondReturned = true;
  first.join();

  return started && !state.timedOut && state.observed == 42;
}

unittest(concurrent_finally_calls_retain_callback_after_first_request_returns) {
  assertTrue(overlappingRequestsRetainCallback(&App::finally));
}

unittest(
    concurrent_not_found_calls_retain_callback_after_first_request_returns) {
  assertTrue(overlappingRequestsRetainCallback(&App::notFound));
}

unittest(concurrent_requests_can_replace_and_clear_app_callbacks) {
  const char* request = "GET / HTTP/1.1" CRLF CRLF;
  const std::string expected(256, 'c');
  std::atomic<bool> start{false};
  std::atomic<bool> timedOut{false};
  std::atomic<bool> corrupted{false};
  std::atomic<int> calls{0};
  App app;
  auto callback = [&, expected](Request&, Response&) {
    if (expected != std::string(256, 'c')) corrupted = true;
    ++calls;
  };
  app.finally(callback);
  app.notFound(callback);
  MockStream initial(request);
  app.process(&initial);

  auto readRequests = [&]() {
    if (!waitForFlag(start)) {
      timedOut = true;
      return;
    }
    for (int i = 0; i < 1000; ++i) {
      MockStream stream(request);
      app.process(&stream);
    }
  };
  std::thread first(readRequests);
  std::thread second(readRequests);
  start = true;
  for (int i = 0; i < 1000; ++i) {
    app.finally(nullptr);
    app.notFound(callback);
    app.finally(callback);
    app.notFound(nullptr);
  }
  first.join();
  second.join();

  assertFalse(timedOut.load());
  assertFalse(corrupted.load());
  assertMore(calls.load(), 0);
}

unittest(throwing_app_callbacks_remain_registered_for_later_requests) {
  const char* request = "GET / HTTP/1.1" CRLF CRLF;
  const AppCallbackSetter setters[] = {&App::finally, &App::notFound};
  for (AppCallbackSetter setter : setters) {
    App app;
    int calls = 0;
    (app.*setter)([&](Request&, Response&) {
      if (++calls == 1) throw std::runtime_error("callback failed");
    });
    bool caught = false;
    MockStream first(request);
    try {
      app.process(&first);
    } catch (const std::runtime_error&) {
      caught = true;
    }
    MockStream later(request);
    app.process(&later);

    assertTrue(caught);
    assertEqual(2, calls);
  }
}

unittest(throwing_app_callbacks_preserve_their_replacements) {
  const char* request = "GET / HTTP/1.1" CRLF CRLF;
  const AppCallbackSetter setters[] = {&App::finally, &App::notFound};
  for (AppCallbackSetter setter : setters) {
    App app;
    int originalCalls = 0;
    int replacementCalls = 0;
    (app.*setter)([&](Request&, Response&) {
      ++originalCalls;
      (app.*setter)([&](Request&, Response&) { ++replacementCalls; });
      throw std::runtime_error("replaced callback failed");
    });
    bool caught = false;
    MockStream first(request);
    try {
      app.process(&first);
    } catch (const std::runtime_error&) {
      caught = true;
    }
    MockStream later(request);
    app.process(&later);

    assertTrue(caught);
    assertEqual(1, originalCalls);
    assertEqual(1, replacementCalls);
  }
}
#endif

unittest_main()
