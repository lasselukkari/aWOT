#include <ArduinoUnitTests.h>
#include <aWOT.h>
#include <stdio.h>

#include "mocks/SharedClient.h"

static void downloadFirmware(Request&, Response& res) {
  static const uint8_t firmware[] =
      "AWOT"
      "\0\x01"
      "0123456789abcdef"
      "0123456789abcdef"
      "\xfe\xff";
  char contentLength[12];
  snprintf(contentLength, sizeof(contentLength), "%u",
           static_cast<unsigned>(sizeof(firmware) - 1));

  res.set("Content-Type", "application/octet-stream");
  res.set("Content-Length", contentLength);
  res.set("Connection", "keep-alive");
  // Use the byte count, since a binary file can contain zero bytes.
  res.write(firmware, sizeof(firmware) - 1);
}

static void downloadMeasurements(Request&, Response& res) {
  static const uint8_t heading[] = "sample,temperature\n";
  static const uint8_t first[] = "1000,21.5\n";
  static const uint8_t second[] = "2000,22.0\n";
  res.set("Content-Type", "text/csv");
  res.set("Connection", "keep-alive");

  // Send each available piece without knowing the final content length.
  res.write(heading, sizeof(heading) - 1);
  res.flush();
  res.write(first, sizeof(first) - 1);
  res.flush();
  res.write(second, sizeof(second) - 1);
  // process() flushes the last piece and terminates the chunked response.
}

unittest(binary_download_larger_than_output_buffer) {
  const char* request = "GET /firmware.bin HTTP/1.1" CRLF CRLF;
  const char expected[] =
      "HTTP/1.1 200 OK" CRLF "Content-Type: application/octet-stream" CRLF
      "Content-Length: 40" CRLF "Connection: keep-alive" CRLF CRLF
      "AWOT"
      "\0\x01"
      "0123456789abcdef"
      "0123456789abcdef"
      "\xfe\xff";

  SharedClient client;
  client.feed(request);
  App app;
  app.get("/firmware.bin", downloadFirmware);
  uint8_t writeBuffer[32];
  App::ProcessOptions options;
  options.writeBuffer = writeBuffer;

  App::ProcessResult result = app.process(&client, options);

  // Construct with a length so the comparison includes bytes after the NUL.
  assertEqual(std::string(expected, sizeof(expected) - 1),
              client.outputString());
  assertTrue(result.keepAlive);
  assertFalse(result.responseOpen);
  assertTrue(client.connected());
}

unittest(chunked_download_flushes_pieces_and_finishes_once) {
  const char* request = "GET /measurements.csv HTTP/1.1" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 200 OK" CRLF "Content-Type: text/csv" CRLF
      "Connection: keep-alive" CRLF "Transfer-Encoding: chunked" CRLF CRLF
      "10" CRLF "sample,temperatu" CRLF "3" CRLF "re\n" CRLF "a" CRLF
      "1000,21.5\n" CRLF "a" CRLF "2000,22.0\n" CRLF "0" CRLF CRLF;

  SharedClient client;
  client.feed(request);
  App app;
  app.get("/measurements.csv", downloadMeasurements);
  // Allow 16 payload bytes plus framing space. The first piece spans two
  // chunks; flush() sends the shorter pieces before the buffer fills.
  uint8_t writeBuffer[CHUNK_OVERHEAD + 16];
  App::ProcessOptions options;
  options.writeBuffer = writeBuffer;

  App::ProcessResult result = app.process(&client, options);

  assertEqual(std::string(expected), client.outputString());
  assertTrue(result.keepAlive);
  assertFalse(result.responseOpen);
  assertTrue(client.connected());
}

unittest_main()
