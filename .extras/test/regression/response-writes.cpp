#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include <string>
#include <vector>

#include "mocks/SharedClient.h"

struct WriteOptions {
  std::string payload;
  bool bytewise;
  bool open;
};

static void writeBody(Request& req, Response& res) {
  WriteOptions& options = *static_cast<WriteOptions*>(req.context);
  res.set("Connection", "keep-alive");
  if (options.open) res.keepOpen();
  if (options.bytewise) {
    for (size_t i = 0; i < options.payload.size(); ++i)
      res.write((uint8_t)options.payload[i]);
  } else {
    res.write((const uint8_t*)options.payload.data(), options.payload.size());
  }
}

// Parse the wire format independently of the library's buffer/chunk constants.
static bool decodeChunks(const std::string& response, std::string& decoded) {
  size_t pos = response.find("\r\n\r\n");
  if (pos == std::string::npos) return false;
  pos += 4;
  decoded.clear();
  for (;;) {
    size_t end = response.find("\r\n", pos);
    if (end == std::string::npos || end == pos) return false;
    std::string header = response.substr(pos, end - pos);
    char* tail;
    unsigned long length = strtoul(header.c_str(), &tail, 16);
    if (*tail) return false;
    pos = end + 2;
    if (length > response.size() - pos) return false;
    decoded.append(response, pos, length);
    pos += length;
    if (response.compare(pos, 2, "\r\n") != 0) return false;
    pos += 2;
    if (!length) return pos == response.size();
  }
}

unittest(custom_output_capacities_preserve_chunked_payload) {
  const int capacities[] = {CHUNK_OVERHEAD + 1, 32, 5000};
  for (unsigned i = 0; i < sizeof(capacities) / sizeof(capacities[0]); ++i) {
    for (int bytewise = 0; bytewise < 2; ++bytewise) {
      SharedClient client;
      client.feed("GET / HTTP/1.1" CRLF CRLF);
      client.state->maxWrite = 7;
      std::vector<uint8_t> storage(capacities[i] + 2, 0xA5);
      WriteOptions write = {std::string(10000, 'x'), bytewise != 0, false};
      App::ProcessOptions options;
      options.writeBuffer = {&storage[1], capacities[i]};
      options.context = &write;
      App app;
      app.get("/", writeBody);
      App::ProcessResult result = app.process(&client, options);
      std::string decoded;
      assertTrue(decodeChunks(client.outputString(), decoded));
      assertTrue(decoded == write.payload);
      assertEqual((uint8_t)0xA5, storage.front());
      assertEqual((uint8_t)0xA5, storage.back());
      assertTrue(result.keepAlive);
      assertFalse(result.responseOpen);
      assertTrue(client.connected());
    }
  }
}

unittest(zero_writes_close_connection_at_every_response_stage) {
  // Small payload stays buffered until finalization with the default buffer;
  // large payload takes the direct write path.
  const size_t sizes[] = {3, 2000};
  for (unsigned s = 0; s < sizeof(sizes) / sizeof(sizes[0]); ++s) {
    for (int open = 0; open < 2; ++open) {
      WriteOptions write = {std::string(sizes[s], 'x'), false, open != 0};
      App::ProcessOptions options;
      options.context = &write;
      App app;
      app.get("/", writeBody);
      SharedClient baseline;
      baseline.feed("GET / HTTP/1.1" CRLF CRLF);
      app.process(&baseline, options);
      const std::string response = baseline.outputString();
      size_t body = response.find("\r\n\r\n") + 4;
      const size_t failures[] = {0,    7,        body - 1,
                                 body, body + 1, response.size() - 1};
      for (unsigned f = 0; f < sizeof(failures) / sizeof(failures[0]); ++f) {
        SharedClient client;
        client.feed("GET / HTTP/1.1" CRLF CRLF);
        client.state->maxWrite = 2;
        client.state->failAfter = failures[f];
        App::ProcessResult result = app.process(&client, options);
        assertFalse(result.keepAlive);
        assertFalse(result.responseOpen);
        assertFalse(client.connected());
        assertTrue(client.outputString() == response.substr(0, failures[f]));
      }
    }
  }
}

unittest(unread_request_body_prevents_connection_reuse) {
  SharedClient client;
  client.feed("GET / HTTP/1.1" CRLF "Content-Length: 3" CRLF CRLF "abc");
  WriteOptions write = {"ok", false, false};
  App::ProcessOptions options;
  options.context = &write;
  App app;
  app.get("/", writeBody);
  App::ProcessResult result = app.process(&client, options);
  assertFalse(result.keepAlive);
  assertFalse(result.responseOpen);
  assertEqual(3, client.available());
}

unittest_main()
