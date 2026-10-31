#include <ArduinoUnitTests.h>
#include <aWOT.h>
#include <stdio.h>

#include "mocks/SharedClient.h"

static void echoUpload(Request& req, Response& res) {
  uint8_t payload[64];
  // readBytes() returns decoded payload, including any embedded zero bytes.
  size_t length = req.readBytes(payload, sizeof(payload));
  // Accept only a complete upload that fits in the buffer. left() also
  // consumes the final chunk and trailers before reporting completion.
  if (req.left() != 0) return res.sendStatus(400);

  char contentLength[12];
  snprintf(contentLength, sizeof(contentLength), "%u",
           static_cast<unsigned>(length));
  res.set("Content-Type", "application/octet-stream");
  res.set("Content-Length", contentLength);
  res.set("Connection", "keep-alive");
  const char* uploadId = req.get("X-Upload-Id");
  if (uploadId) res.set("X-Upload-Id", uploadId);
  // Emit the response even when the upload has no payload.
  res.endHeaders();
  res.write(payload, length);
}

static void uploadStatus(Request&, Response& res) { res.print("ready"); }

unittest(chunked_upload_decodes_binary_payload_extensions_and_trailers) {
  const char request[] =
      "POST /upload HTTP/1.1" CRLF "Content-Type: application/octet-stream" CRLF
      "Transfer-Encoding: chunked" CRLF
      "Trailer: X-Checksum, X-Upload-Note" CRLF CRLF "10;tag=\"x;y\"" CRLF
      "0123456789abcdef" CRLF "0a \t;part=last" CRLF
      "01234\0\xff"
      "789" CRLF "0;done" CRLF "X-Checksum: example" CRLF
      "X-Upload-Note: complete" CRLF CRLF;
  const char expected[] =
      "HTTP/1.1 200 OK" CRLF "Content-Type: application/octet-stream" CRLF
      "Content-Length: 26" CRLF "Connection: keep-alive" CRLF CRLF
      "0123456789abcdef01234\0\xff"
      "789";

  SharedClient client;
  client.feed(std::string(request, sizeof(request) - 1));
  App app;
  app.setTimeout(0);
  app.post("/upload", echoUpload);

  App::ProcessResult result = app.process(&client);

  assertEqual(std::string(expected, sizeof(expected) - 1),
              client.outputString());
  assertTrue(result.keepAlive);
  assertFalse(result.responseOpen);
  assertTrue(client.connected());
  assertEqual(0, client.available());
}

unittest(empty_chunked_upload_returns_a_complete_success_response) {
  const char* request = "POST /upload HTTP/1.1" CRLF
                        "Transfer-Encoding: chunked" CRLF CRLF "0" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 200 OK" CRLF "Content-Type: application/octet-stream" CRLF
      "Content-Length: 0" CRLF "Connection: keep-alive" CRLF CRLF;

  SharedClient client;
  client.feed(request);
  App app;
  app.setTimeout(0);
  app.post("/upload", echoUpload);

  App::ProcessResult result = app.process(&client);

  assertEqual(std::string(expected), client.outputString());
  assertTrue(result.keepAlive);
  assertFalse(result.responseOpen);
  assertTrue(client.connected());
  assertEqual(0, client.available());
}

unittest(chunked_upload_trailers_leave_next_request_ready_for_reuse) {
  const char* upload =
      "POST /upload HTTP/1.1" CRLF "Transfer-Encoding: chunked" CRLF
      "Trailer: X-Checksum" CRLF CRLF "5;part=first" CRLF "hello" CRLF "0" CRLF
      "X-Checksum: example" CRLF CRLF;
  const char* next = "GET /status HTTP/1.1" CRLF "Connection: close" CRLF CRLF;
  const char* expectedUpload =
      "HTTP/1.1 200 OK" CRLF "Content-Type: application/octet-stream" CRLF
      "Content-Length: 5" CRLF "Connection: keep-alive" CRLF CRLF "hello";
  const char* expectedStatus =
      "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "ready";

  SharedClient client;
  client.feed(std::string(upload) + next);
  App app;
  app.setTimeout(0);
  app.post("/upload", echoUpload);
  app.get("/status", uploadStatus);

  App::ProcessResult first = app.process(&client);

  assertEqual(std::string(expectedUpload), client.outputString());
  assertTrue(first.keepAlive);
  assertFalse(first.responseOpen);
  assertTrue(client.connected());
  assertEqual(static_cast<int>(strlen(next)), client.available());
  assertEqual(static_cast<int>('G'), client.peek());

  // The server loop processes another request only when reuse is allowed.
  App::ProcessResult second;
  if (first.keepAlive) second = app.process(&client);
  if (!second.keepAlive) client.stop();

  assertEqual(std::string(expectedUpload) + expectedStatus,
              client.outputString());
  assertFalse(second.keepAlive);
  assertFalse(second.responseOpen);
  assertFalse(client.connected());
  assertEqual(0, client.available());
}

unittest(chunked_upload_rejects_malformed_sizes_extensions_and_trailers) {
  const char* headers =
      "POST /upload HTTP/1.1" CRLF "Transfer-Encoding: chunked" CRLF
      "Connection: keep-alive" CRLF CRLF;
  const char* bodies[] = {
      "Z" CRLF "x" CRLF "0" CRLF CRLF,  // Invalid hexadecimal size.
      "100000000" CRLF,                 // Size exceeds 32 bits.
      "1;tag=\x01" CRLF "x" CRLF "0" CRLF CRLF,
      "1" CRLF "x" CRLF "0" CRLF "Bad Header: value" CRLF CRLF,
      "1" CRLF "x" CRLF "0" CRLF "X-Checksum: bad\x01" CRLF CRLF,
  };
  const char* expected =
      "HTTP/1.1 400 Bad Request" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Bad Request";

  for (const char* body : bodies) {
    SharedClient client;
    client.feed(std::string(headers) + body);
    App app;
    app.setTimeout(0);
    app.post("/upload", echoUpload);

    App::ProcessResult result = app.process(&client);

    // No successful upload response may precede the error response.
    assertEqual(std::string(expected), client.outputString());
    assertFalse(result.keepAlive);
    assertFalse(result.responseOpen);
    assertFalse(client.connected());
  }
}

unittest(chunked_upload_trailers_do_not_replace_captured_headers) {
  const char* request =
      "POST /upload HTTP/1.1" CRLF "Transfer-Encoding: chunked" CRLF
      "X-Upload-Id: original" CRLF "Trailer: X-Upload-Id" CRLF CRLF "5" CRLF
      "hello" CRLF "0" CRLF "X-Upload-Id: replaced" CRLF CRLF;
  const char* expected =
      "HTTP/1.1 200 OK" CRLF "Content-Type: application/octet-stream" CRLF
      "Content-Length: 5" CRLF "Connection: keep-alive" CRLF
      "X-Upload-Id: original" CRLF CRLF "hello";

  SharedClient client;
  client.feed(request);
  App app;
  app.setTimeout(0);
  app.post("/upload", echoUpload);
  char uploadId[16];
  Request::Header headers[] = {{"X-Upload-Id", uploadId}};
  App::ProcessOptions options;
  options.headers = headers;

  App::ProcessResult result = app.process(&client, options);

  // The handler reads the captured header after consuming all trailers.
  assertEqual(std::string(expected), client.outputString());
  assertTrue(result.keepAlive);
  assertFalse(result.responseOpen);
  assertTrue(client.connected());
  assertEqual(0, client.available());
}

unittest_main()
