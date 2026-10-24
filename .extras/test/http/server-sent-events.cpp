#include <ArduinoUnitTests.h>
#include <SseChannel.h>
#include <aWOT.h>

#include "mocks/SharedClient.h"

using Events = awot::SseChannel<SharedClient, 2>;

static void keepAlive(Request& req, Response& res) {
  res.set("Connection", "keep-alive");
  req.next();
}

static void openEvents(Request& req, Response& res) {
  Events& events = *static_cast<Events*>(req.context);
  events.open(res);
}

unittest(sse_open_after_keepalive_sends_close_delimited_events) {
  const char* request = "GET /events HTTP/1.1" CRLF "Host: local" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 200 OK" CRLF "Content-Type: text/event-stream" CRLF
      "Cache-Control: no-cache" CRLF "Connection: close" CRLF CRLF
      "data: hello\n\n";

  Events events;
  SharedClient client;
  client.feed(request);
  App app;
  app.use(keepAlive);
  app.get("/events", openEvents);

  App::ProcessOptions options;
  options.context = &events;
  App::ProcessResult result = app.process(&client, options);
  assertTrue(result.responseOpen);
  assertFalse(result.keepAlive);

  // Retain the client only after the HTTP response has opened successfully.
  if (result.responseOpen) {
    events.add(client);
  } else {
    client.stop();
  }
  events.broadcast("hello");

  assertEqual(std::string(expected), client.outputString());
}

unittest_main()
