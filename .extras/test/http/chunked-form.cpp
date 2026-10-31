#include <ArduinoUnitTests.h>
#include <aWOT.h>

#include "mocks/MockStream.h"

static void submitForm(Request& req, Response& res) {
  char field[8];
  char value[16];
  char name[16] = {};
  char note[16] = {};

  // Chunked bodies have an unknown length until the final chunk is read.
  // Stop immediately on a failed form read, including an incomplete body.
  while (req.left() != 0) {
    if (!req.form(field, sizeof(field), value, sizeof(value))) {
      return res.sendStatus(400);
    }

    if (strcmp(field, "name") == 0) {
      strcpy(name, value);
    } else if (strcmp(field, "note") == 0) {
      strcpy(note, value);
    } else {
      return res.sendStatus(400);
    }
  }

  if (!name[0] || !note[0]) return res.sendStatus(400);

  // Write the response only after the complete form has been validated.
  res.print("name: ");
  res.println(name);
  res.print("note: ");
  res.println(note);
}

unittest(chunked_form_decodes_fields_and_percent_escapes_across_chunks) {
  // The payload is name=Jo%73e+Doe&note=A%26B. Both escapes span chunks.
  const char* request =
      "POST /form HTTP/1.1" CRLF
      "Content-Type: application/x-www-form-urlencoded" CRLF
      "Transfer-Encoding: chunked" CRLF CRLF "5" CRLF "name=" CRLF "4" CRLF
      "Jo%7" CRLF "9" CRLF "3e+Doe&no" CRLF "6" CRLF "te=A%2" CRLF "2" CRLF
      "6B" CRLF "0" CRLF CRLF;

  const char* expected =
      "HTTP/1.1 200 OK" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "name: Jose Doe" CRLF "note: A&B" CRLF;

  MockStream stream(request);
  App app;

  app.post("/form", submitForm);
  app.process(&stream);

  assertEqual(expected, stream.response());
}

unittest(chunked_form_rejects_invalid_percent_escape) {
  const char* request = "POST /form HTTP/1.1" CRLF
                        "Content-Type: application/x-www-form-urlencoded" CRLF
                        "Transfer-Encoding: chunked" CRLF CRLF "9" CRLF
                        "name=A%QZ" CRLF "0" CRLF CRLF;

  const char* expected =
      "HTTP/1.1 400 Bad Request" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Bad Request";

  MockStream stream(request);
  App app;

  app.post("/form", submitForm);
  App::ProcessResult result = app.process(&stream);

  assertEqual(expected, stream.response());
  assertFalse(result.keepAlive);
}

unittest(chunked_form_rejects_malformed_chunk_delimiter) {
  const char* request = "POST /form HTTP/1.1" CRLF
                        "Content-Type: application/x-www-form-urlencoded" CRLF
                        "Transfer-Encoding: chunked" CRLF CRLF "6" CRLF
                        "name=A!" CRLF "0" CRLF CRLF;

  const char* expected =
      "HTTP/1.1 400 Bad Request" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Bad Request";

  MockStream stream(request);
  App app;

  app.post("/form", submitForm);
  App::ProcessResult result = app.process(&stream);

  assertEqual(expected, stream.response());
  assertFalse(result.keepAlive);
}

unittest(chunked_form_rejects_missing_final_chunk) {
  const char* request = "POST /form HTTP/1.1" CRLF
                        "Content-Type: application/x-www-form-urlencoded" CRLF
                        "Transfer-Encoding: chunked" CRLF CRLF "6" CRLF
                        "name=A" CRLF "7" CRLF "&note=B" CRLF;

  const char* expected =
      "HTTP/1.1 400 Bad Request" CRLF "Content-Type: text/plain" CRLF
      "Connection: close" CRLF CRLF "Bad Request";

  MockStream stream(request);
  App app;
  app.setTimeout(0);

  app.post("/form", submitForm);
  App::ProcessResult result = app.process(&stream);

  assertEqual(expected, stream.response());
  assertFalse(result.keepAlive);
}

unittest_main()
