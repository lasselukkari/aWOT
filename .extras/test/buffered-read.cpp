#include <ArduinoUnitTests.h>
#include <limits.h>

#include "../../src/aWOT.h"
#include "./mocks/MockClient.h"
#include "./mocks/MockStream.h"

void handler(Request& req, Response& res) {
  uint8_t buffer[100] = {};

  int read = req.read(buffer, 100);

  res.write(buffer, read);
}

void readCountHandler(Request& req, Response& res) {
  uint8_t buffer[100] = {};
  req.push(req.read());

  int read = req.read(buffer, 100);

  res.print(read);
}

unittest(buffered_read) {
  const char* request =
      "POST / HTTP/1.0" CRLF "Content-Length: 4" CRLF CRLF "body";

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "4";

  MockStream stream(request);
  App app;

  app.post("/", &readCountHandler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(buffered_read_count) {
  const char* request =
      "POST / HTTP/1.0" CRLF "Content-Length: 4" CRLF CRLF "body";

  const char* expected = "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
                         "Connection: close" CRLF CRLF "body";

  MockStream stream(request);
  App app;

  app.post("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

// Report generated body bytes without allocating or storing INT_MAX bytes.
class GeneratedBodyClient : public MockClient {
 public:
  GeneratedBodyClient() : remaining(INT_MAX) {
    char header[80];
    snprintf(header, sizeof(header),
             "POST / HTTP/1.1" CRLF "Content-Length: %d" CRLF CRLF, INT_MAX);
    setReadBuffer((const uint8_t*)header, strlen(header));
  }

  int read() override {
    if (MockClient::available()) return MockClient::read();
    if (!remaining) return -1;
    --remaining;
    return 'X';
  }

  int read(uint8_t* buffer, size_t size) override {
    int count = (int)min(size, (size_t)remaining);
    if (count) *buffer = 'X';
    remaining -= count;
    return count;
  }

 private:
  int remaining;
};

struct CounterObservation {
  bool bytewise;
  int before;
  int boundary;
  int after;
  int left;
};

void observeCounterSaturation(Request& req, Response& res) {
  CounterObservation& observation =
      *static_cast<CounterObservation*>(req.context);
  uint8_t byte;
  int remaining = INT_MAX - req.bytesRead() - 1;
  if (req.read(&byte, (size_t)remaining) != remaining)
    return res.sendStatus(500);
  observation.before = req.bytesRead();
  if (observation.bytewise) {
    req.read();
    observation.boundary = req.bytesRead();
    req.read();
  } else {
    req.read(&byte, 2);
    observation.boundary = req.bytesRead();
  }
  if (req.read(&byte, (size_t)req.left()) <= 0) return res.sendStatus(500);
  observation.after = req.bytesRead();
  observation.left = req.left();
  res.sendStatus(200);
}

unittest(request_byte_counter_saturates_for_scalar_and_buffered_reads) {
  for (int bytewise = 0; bytewise < 2; ++bytewise) {
    GeneratedBodyClient client;
    CounterObservation observation = {bytewise != 0, 0, 0, 0, -1};
    App::ProcessOptions options;
    options.context = &observation;
    App app;
    app.post("/", observeCounterSaturation);
    app.process(&client, options);

    assertEqual(INT_MAX - 1, observation.before);
    assertEqual(INT_MAX, observation.boundary);
    assertEqual(INT_MAX, observation.after);
    assertEqual(0, observation.left);
  }
}

unittest_main()
