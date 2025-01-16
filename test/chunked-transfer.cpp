#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

#define READ_BUFFER_SIZE 16

void handler(Request & req, Response & res) {

  byte readBuffer[READ_BUFFER_SIZE];

  // Mocked file content
  MockStream dataFile("To be, or not to be, that is the question.");

  uint32_t length = dataFile.available();

  res.set("Connection", "keep-alive"); // Force Transfer-Encoding: chunked

  while (length > 0) {
    int toRead = length > READ_BUFFER_SIZE ? READ_BUFFER_SIZE : length;
    dataFile.readBytes(readBuffer, toRead);
    // Implicit call to new 'Response::write(const uint8_t *buffer, size_t size)' method
    // for SERVER_OUTPUT_BUFFER_SIZE-sized chunks response
    res.write(readBuffer, toRead);
    length = length - toRead;
  }
  res.end();
}

void handlerBufferSize(Request & req, Response & res) {

  byte readBuffer[READ_BUFFER_SIZE];

  // Mocked file content
  MockStream dataFile("To be, or not to be, that is the question.");

  uint32_t length = dataFile.available();

  res.set("Connection", "keep-alive"); // Force Transfer-Encoding: chunked

  while (length > 0) {
    int toRead = length > READ_BUFFER_SIZE ? READ_BUFFER_SIZE : length;
    dataFile.readBytes(readBuffer, toRead);
    // Call to implemented 'Response::writeF(uint8_t *buffer, size_t bufferLength)' method
    // for readBuffer-sized chunks response
    res.writeF(readBuffer, toRead);
    length = length - toRead;
  }
  res.end();
}

unittest(transfer_encoding_chunked) {
  const char *request =
    "GET / HTTP/1.1" CRLF
    CRLF;

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Connection: keep-alive" CRLF
    "Content-Type: text/plain" CRLF
    "Transfer-Encoding: chunked" CRLF
    CRLF
    "2A" CRLF
    "To be, or not to be, that is the question." CRLF
    "0" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.get("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(transfer_encoding_chunked_buffer_size_16) {
  const char *request =
    "GET / HTTP/1.1" CRLF
    CRLF;

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Connection: keep-alive" CRLF
    "Content-Type: text/plain" CRLF
    "Transfer-Encoding: chunked" CRLF
    CRLF
    "10" CRLF
    "To be, or not to" CRLF
    "10" CRLF
    " be, that is the" CRLF
    "A" CRLF
    " question." CRLF
    "0" CRLF
    CRLF;

  MockStream stream(request);
  Application app;

  app.get("/", &handlerBufferSize);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
