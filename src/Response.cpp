/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things
*/

#include "Response.h"

#include "App.h"

using namespace awot;

Response::Response(Client* client, uint8_t* writeBuffer, int writeBufferLength)
    : m_stream(client),
      m_headers(),
      m_contentLengthSet(false),
      m_contentTypeSet(false),
      m_keepAlive(false),
      m_statusSent(0),
      m_headersSent(false),
      m_sendingStatus(false),
      m_sendingHeaders(false),
      m_headersCount(0),
      m_bytesSent(0),
      m_ended(false),
      m_buffer(writeBuffer),
      m_bufferLength(writeBufferLength),
      m_bufFill(0) {}

int Response::availableForWrite() { return CHUNK_DATA_SIZE - m_bufFill - 1; }

void Response::beginHeaders() {
  if (!m_statusSent) {
    status(200);
  }

  m_sendingHeaders = true;

  P(headerSeparator) = ": ";
  for (int i = 0; i < m_headersCount; i++) {
    this->print(m_headers[i].name);
    printP(headerSeparator);
    this->print(m_headers[i].value);
    m_printCRLF();
  }
}

int Response::bytesSent() { return m_bytesSent; }

void Response::end() { m_ended = true; }

void Response::endHeaders() {
  m_printCRLF();
  m_flushBuf();
  m_sendingHeaders = false;
  m_headersSent = true;
}

bool Response::ended() { return m_ended; }

void Response::flush() {
  m_flushBuf();

  m_stream->flush();
}

const char* Response::get(const char* name) {
  for (int i = 0; i < m_headersCount; i++) {
    if (App::strcmpi(name, m_headers[i].name) == 0) {
      return m_headers[i].value;
    }
  }

  return NULL;
}

bool Response::headersSent() { return m_headersSent; }

void Response::keepOpen() { m_responseOpen = true; }

void Response::printP(const unsigned char* string) {
  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  while (uint8_t value = pgm_read_byte(string++)) {
    write(value);
  }
}

void Response::printP(const char* string) { printP((unsigned char*)string); }

void Response::sendStatus(int code) {
  status(code);

  m_printHeaders();

  if (code != 204 && code != 304) {
    m_printStatus(code);
  }
}

void Response::set(const char* name, const char* value) {
  if (m_headersCount >= SERVER_MAX_HEADERS) {
    return;
  }

  m_headers[m_headersCount].name = name;
  m_headers[m_headersCount].value = value;
  m_headersCount++;

  P(contentType) = "Content-Type";
  if (App::strcmpiP(name, contentType) == 0) {
    m_contentTypeSet = true;
  }

  P(contentLength) = "Content-Length";
  if (App::strcmpiP(name, contentLength) == 0) {
    m_contentLengthSet = true;
  }

  P(connection) = "Connection";
  if (App::strcmpiP(name, connection) == 0) {
    P(keepAlive) = "keep-alive";
    m_keepAlive = App::strcmpiP(value, keepAlive) == 0;
  }
}

void Response::setDefaults() {
  if (!m_contentTypeSet) {
    set("Content-Type", "text/plain");
  }

  if (m_keepAlive && !m_contentLengthSet) {
    set("Transfer-Encoding", "chunked");
  }

  if (!m_keepAlive) {
    m_contentLengthSet = true;
    set("Connection", "close");
  }
}

void Response::status(int code) {
  if (m_statusSent) {
    return;
  }

  m_statusSent = code;

  m_sendingStatus = true;
  P(httpVersion) = "HTTP/1.1 ";
  printP(httpVersion);
  this->print(code);
  P(space) = " ";
  printP(space);
  m_printStatus(code);

  m_printCRLF();

  if (code < 200) {
    beginHeaders();
    endHeaders();
    m_statusSent = 0;
  } else if (code == 204 || code == 304) {
    m_contentLengthSet = true;
    m_contentTypeSet = true;
  }

  m_sendingStatus = false;
}

int Response::statusSent() { return m_statusSent; }

size_t Response::write(uint8_t data) {
  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  // Data is written starting after the reserved header space
  m_buffer[CHUNK_HEADER_SIZE + m_bufFill++] = data;

  if (m_bufFill == CHUNK_DATA_SIZE) {
    m_flushBuf();
  }

  size_t bytesSent = sizeof(data);
  m_bytesSent += bytesSent;
  return bytesSent;
}

size_t Response::write(const uint8_t* buffer, size_t bufferLength) {
  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  if (m_bufFill + bufferLength < (size_t)CHUNK_DATA_SIZE) {
    // Data fits in buffer - copy after header space
    memcpy(m_buffer + CHUNK_HEADER_SIZE + m_bufFill, buffer, bufferLength);
    m_bufFill += bufferLength;
  } else {
    // Flush current buffer first
    m_flushBuf();

    // Write large buffer directly with chunk encoding
    if (m_headersSent && !m_contentLengthSet) {
      // Write the chunk header to m_buffer, then send separately
      int headerLen = m_writeChunkHeader((int)bufferLength);
      m_stream->write((uint8_t*)m_buffer + CHUNK_HEADER_SIZE - headerLen,
                      headerLen);
      m_stream->write(buffer, bufferLength);
      m_stream->write((const uint8_t*)"\r\n", 2);
    } else {
      m_stream->write(buffer, bufferLength);
    }
  }

  m_bytesSent += bufferLength;
  return bufferLength;
}

void Response::writeP(const unsigned char* data, size_t length) {
  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  while (length--) {
    write(pgm_read_byte(data++));
  }
}

void Response::m_printStatus(int code) {
  switch (code) {
#ifndef LOW_MEMORY_MCU
    case 100: {
      P(Continue) = "Continue";
      printP(Continue);
      break;
    }
    case 101: {
      P(SwitchingProtocols) = "Switching Protocols";
      printP(SwitchingProtocols);
      break;
    }
    case 102: {
      P(Processing) = "Processing";
      printP(Processing);
      break;
    }
    case 103: {
      P(EarlyHints) = "Early Hints";
      printP(EarlyHints);
      break;
    }
    case 200: {
      P(OK) = "OK";
      printP(OK);
      break;
    }
    case 201: {
      P(Created) = "Created";
      printP(Created);
      break;
    }
    case 202: {
      P(Accepted) = "Accepted";
      printP(Accepted);
      break;
    }
    case 203: {
      P(NonAuthoritativeInformation) = "Non-Authoritative Information";
      printP(NonAuthoritativeInformation);
      break;
    }
    case 204: {
      P(NoContent) = "No Content";
      printP(NoContent);
      break;
    }
    case 205: {
      P(ResetContent) = "Reset Content";
      printP(ResetContent);
      break;
    }
    case 206: {
      P(PartialContent) = "Partial Content";
      printP(PartialContent);
      break;
    }
    case 207: {
      P(MultiStatus) = "Multi-Status";
      printP(MultiStatus);
      break;
    }
    case 208: {
      P(AlreadyReported) = "Already Reported";
      printP(AlreadyReported);
      break;
    }
    case 226: {
      P(IMUsed) = "IM Used";
      printP(IMUsed);
      break;
    }
    case 300: {
      P(MultipleChoices) = "Multiple Choices";
      printP(MultipleChoices);
      break;
    }
    case 301: {
      P(MovedPermanently) = "Moved Permanently";
      printP(MovedPermanently);
      break;
    }
    case 302: {
      P(Found) = "Found";
      printP(Found);
      break;
    }
    case 303: {
      P(SeeOther) = "See Other";
      printP(SeeOther);
      break;
    }
    case 304: {
      P(NotModified) = "Not Modified";
      printP(NotModified);
      break;
    }
    case 305: {
      P(UseProxy) = "Use Proxy";
      printP(UseProxy);
      break;
    }
    case 306: {
      P(Unused) = "(Unused)";
      printP(Unused);
      break;
    }
    case 307: {
      P(TemporaryRedirect) = "Temporary Redirect";
      printP(TemporaryRedirect);
      break;
    }
    case 308: {
      P(PermanentRedirect) = "Permanent Redirect";
      printP(PermanentRedirect);
      break;
    }
    case 400: {
      P(BadRequest) = "Bad Request";
      printP(BadRequest);
      break;
    }
    case 401: {
      P(Unauthorized) = "Unauthorized";
      printP(Unauthorized);
      break;
    }
    case 402: {
      P(PaymentRequired) = "Payment Required";
      printP(PaymentRequired);
      break;
    }
    case 403: {
      P(Forbidden) = "Forbidden";
      printP(Forbidden);
      break;
    }
    case 404: {
      P(NotFound) = "Not Found";
      printP(NotFound);
      break;
    }
    case 405: {
      P(MethodNotAllowed) = "Method Not Allowed";
      printP(MethodNotAllowed);
      break;
    }
    case 406: {
      P(NotAcceptable) = "Not Acceptable";
      printP(NotAcceptable);
      break;
    }
    case 407: {
      P(ProxyAuthenticationRequired) = "Proxy Authentication Required";
      printP(ProxyAuthenticationRequired);
      break;
    }
    case 408: {
      P(RequestTimeout) = "Request Timeout";
      printP(RequestTimeout);
      break;
    }
    case 409: {
      P(Conflict) = "Conflict";
      printP(Conflict);
      break;
    }
    case 410: {
      P(Gone) = "Gone";
      printP(Gone);
      break;
    }
    case 411: {
      P(LengthRequired) = "Length Required";
      printP(LengthRequired);
      break;
    }
    case 412: {
      P(PreconditionFailed) = "Precondition Failed";
      printP(PreconditionFailed);
      break;
    }
    case 413: {
      P(PayloadTooLarge) = "Payload Too Large";
      printP(PayloadTooLarge);
      break;
    }
    case 414: {
      P(URITooLong) = "URI Too Long";
      printP(URITooLong);
      break;
    }
    case 415: {
      P(UnsupportedMediaType) = "Unsupported Media Type";
      printP(UnsupportedMediaType);
      break;
    }
    case 416: {
      P(RangeNotSatisfiable) = "Range Not Satisfiable";
      printP(RangeNotSatisfiable);
      break;
    }
    case 417: {
      P(ExpectationFailed) = "Expectation Failed";
      printP(ExpectationFailed);
      break;
    }
    case 421: {
      P(MisdirectedRequest) = "Misdirected Request";
      printP(MisdirectedRequest);
      break;
    }
    case 422: {
      P(UnprocessableEntity) = "Unprocessable Entity";
      printP(UnprocessableEntity);
      break;
    }
    case 423: {
      P(Locked) = "Locked";
      printP(Locked);
      break;
    }
    case 424: {
      P(FailedDependency) = "Failed Dependency";
      printP(FailedDependency);
      break;
    }
    case 425: {
      P(TooEarly) = "Too Early";
      printP(TooEarly);
      break;
    }
    case 426: {
      P(UpgradeRequired) = "Upgrade Required";
      printP(UpgradeRequired);
      break;
    }
    case 428: {
      P(PreconditionRequired) = "Precondition Required";
      printP(PreconditionRequired);
      break;
    }
    case 429: {
      P(TooManyRequests) = "Too Many Requests";
      printP(TooManyRequests);
      break;
    }
    case 431: {
      P(RequestHeaderFieldsTooLarge) = "Request Header Fields Too Large";
      printP(RequestHeaderFieldsTooLarge);
      break;
    }
    case 451: {
      P(UnavailableForLegalReasons) = "Unavailable For Legal Reasons";
      printP(UnavailableForLegalReasons);
      break;
    }
    case 500: {
      P(InternalServerError) = "Internal Server Error";
      printP(InternalServerError);
      break;
    }
    case 501: {
      P(NotImplemented) = "Not Implemented";
      printP(NotImplemented);
      break;
    }
    case 502: {
      P(BadGateway) = "Bad Gateway";
      printP(BadGateway);
      break;
    }
    case 503: {
      P(ServiceUnavailable) = "Service Unavailable";
      printP(ServiceUnavailable);
      break;
    }
    case 504: {
      P(GatewayTimeout) = "Gateway Timeout";
      printP(GatewayTimeout);
      break;
    }
    case 505: {
      P(HTTPVersionNotSupported) = "HTTP Version Not Supported";
      printP(HTTPVersionNotSupported);
      break;
    }
    case 506: {
      P(VariantAlsoNegotiates) = "Variant Also Negotiates";
      printP(VariantAlsoNegotiates);
      break;
    }
    case 507: {
      P(InsufficientStorage) = "Insufficient Storage";
      printP(InsufficientStorage);
      break;
    }
    case 508: {
      P(LoopDetected) = "Loop Detected";
      printP(LoopDetected);
      break;
    }
    case 510: {
      P(NotExtended) = "Not Extended";
      printP(NotExtended);
      break;
    }
    case 511: {
      P(NetworkAuthenticationRequired) = "Network Authentication Required";
      printP(NetworkAuthenticationRequired);
      break;
    }
#else
    case 200: {
      P(OK) = "OK";
      printP(OK);
      break;
    }
    case 201: {
      P(Created) = "Created";
      printP(Created);
      break;
    }
    case 202: {
      P(Accepted) = "Accepted";
      printP(Accepted);
      break;
    }
    case 204: {
      P(NoContent) = "No Content";
      printP(NoContent);
      break;
    }
    case 303: {
      P(SeeOther) = "See Other";
      printP(SeeOther);
      break;
    }
    case 304: {
      P(NotModified) = "Not Modified";
      printP(NotModified);
      break;
    }
    case 400: {
      P(BadRequest) = "Bad Request";
      printP(BadRequest);
      break;
    }
    case 401: {
      P(Unauthorized) = "Unauthorized";
      printP(Unauthorized);
      break;
    }
    case 402: {
      P(PaymentRequired) = "Payment Required";
      printP(PaymentRequired);
      break;
    }
    case 403: {
      P(Forbidden) = "Forbidden";
      printP(Forbidden);
      break;
    }
    case 404: {
      P(NotFound) = "Not Found";
      printP(NotFound);
      break;
    }
    case 405: {
      P(MethodNotAllowed) = "Method Not Allowed";
      printP(MethodNotAllowed);
      break;
    }
    case 406: {
      P(NotAcceptable) = "Not Acceptable";
      printP(NotAcceptable);
      break;
    }
    case 407: {
      P(ProxyAuthenticationRequired) = "Proxy Authentication Required";
      printP(ProxyAuthenticationRequired);
      break;
    }
    case 408: {
      P(RequestTimeout) = "Request Timeout";
      printP(RequestTimeout);
      break;
    }

    case 431: {
      P(RequestHeaderFieldsTooLarge) = "Request Header Fields Too Large";
      printP(RequestHeaderFieldsTooLarge);
      break;
    }
    case 500: {
      P(InternalServerError) = "Internal Server Error";
      printP(InternalServerError);
      break;
    }
    case 505: {
      P(HTTPVersionNotSupported) = "HTTP Version Not Supported";
      printP(HTTPVersionNotSupported);
      break;
    }
#endif
    default: {
      print(code);
      break;
    }
  }
}

bool Response::m_shouldPrintHeaders() {
  return (!m_headersSent && !m_sendingHeaders && !m_sendingStatus);
}

void Response::m_printHeaders() {
  setDefaults();
  beginHeaders();
  endHeaders();
}

void Response::m_printCRLF() { print(CRLF); }

void Response::m_flushBuf() {
  if (m_bufFill > 0) {
    if (m_headersSent && !m_contentLengthSet) {
      // Write chunk header into reserved space at start of buffer
      int headerLen = m_writeChunkHeader(m_bufFill);

      // Write trailer at end of data
      m_buffer[CHUNK_HEADER_SIZE + m_bufFill] = '\r';
      m_buffer[CHUNK_HEADER_SIZE + m_bufFill + 1] = '\n';

      // Single write: header + data + trailer
      int startOffset = CHUNK_HEADER_SIZE - headerLen;
      m_stream->write(m_buffer + startOffset,
                      headerLen + m_bufFill + CHUNK_TRAILER_SIZE);
    } else {
      // No chunking - just write the data portion
      m_stream->write(m_buffer + CHUNK_HEADER_SIZE, m_bufFill);
    }

    m_bufFill = 0;
  }
}

void Response::m_finalize() {
  m_flushBuf();

  if (!m_responseOpen && m_headersSent && !m_contentLengthSet) {
    // Write final chunk: "0\r\n\r\n"
    m_stream->write((const uint8_t*)"0\r\n\r\n", 5);
  }
}

// Write chunk size in hex to the reserved header space (right-aligned)
// Returns the number of characters written (excluding CRLF which is added
// after)
int Response::m_writeChunkHeader(int dataSize) {
  static const char hexDigits[] = "0123456789abcdef";

  // Position to write the CRLF at end of header space
  m_buffer[CHUNK_HEADER_SIZE - 2] = '\r';
  m_buffer[CHUNK_HEADER_SIZE - 1] = '\n';

  // Write hex digits right-to-left before the CRLF
  int pos = CHUNK_HEADER_SIZE - 3;
  int hexLen = 0;
  int n = dataSize;

  do {
    m_buffer[pos--] = hexDigits[n & 0xF];
    n >>= 4;
    hexLen++;
  } while (n > 0 && pos >= 0);

  return hexLen + 2;  // hex digits + CRLF
}
