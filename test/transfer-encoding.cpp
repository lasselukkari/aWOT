#include <ArduinoUnitTests.h>
#include "../src/aWOT.h"
#include "./mocks/MockStream.h"

void handler(Request & req, Response & res) {
  if(req.chunked()){
    res.println("chunked");
  }

  if(req.compressed()){
    res.println("compressed");
  }

  if(req.deflated()){
    res.println("deflated");
  }

  if(req.gzipped()){
    res.println("gzipped");
  }

  while(req.left()){
    res.write(req.read());
  }
}

unittest(tranfer_encoding_single) {
  const char *request =
    "PUT / HTTP/1.0" CRLF
    "Transfer-Encoding: gzip" CRLF
    "Transfer-Encoding: chunked" CRLF   
    CRLF;

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "chunked" CRLF
    "gzipped" CRLF;

  MockStream stream(request);
  Application app;
 
  app.put("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(tranfer_encoding_multiple) {
  const char *request =
    "PUT / HTTP/1.0" CRLF
    "Transfer-Encoding: deflated, gzipped, compressed, chunked" CRLF
    CRLF;

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "chunked" CRLF
    "compressed" CRLF
    "deflated" CRLF
    "gzipped" CRLF;

  MockStream stream(request);
  Application app;
 
  app.put("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}


unittest(chunked_body) {
 const char *request =
    "PUT / HTTP/1.0" CRLF
    "Transfer-Encoding: chunked" CRLF
    CRLF
    "5" CRLF
    "test " CRLF
    "10" CRLF
    "chunked body hex" CRLF
    "0" CRLF
    CRLF;

  const char *expected =
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/plain" CRLF
    "Connection: close" CRLF
    CRLF
    "chunked" CRLF
    "test chunked body hex";

  MockStream stream(request);
  Application app;
 
  app.put("/", &handler);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest_main()
