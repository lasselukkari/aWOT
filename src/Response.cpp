#include "Response.h"

#include <limits.h>

#include "App.h"

using namespace awot;

Response::Response(Client* client, uint8_t* writeBuffer, int writeBufferLength)
    : m_stream(client),
      m_headers(),
      m_contentLengthSet(false),
      m_chunkedEncoding(false),
      m_contentTypeSet(false),
      m_keepAlive(false),
      m_requestKeepAlive(false),
      m_chunkingAllowed(false),
      m_responseOpen(false),
      m_head(false),
      m_statusSent(0),
      m_headersSent(false),
      m_sendingStatus(false),
      m_sendingHeaders(false),
      m_headersCount(0),
      m_bytesSent(0),
      m_ended(false),
      m_buffer(writeBuffer),
      m_bufferLength(writeBufferLength),
      m_bufFill(0) {
  if (!m_buffer || m_capacity() <= 0) m_failWrite();
}

int Response::m_capacity() const {
  if (m_bufferLength <= CHUNK_OVERHEAD) return 0;
  int chunkLimit = 0;
  for (int i = 0; i < CHUNK_HEADER_SIZE - 2; ++i) {
    chunkLimit =
        chunkLimit > (INT_MAX - 15) / 16 ? INT_MAX : chunkLimit * 16 + 15;
  }
  return min(m_bufferLength - CHUNK_OVERHEAD, chunkLimit);
}

int Response::availableForWrite() {
  return getWriteError() ? 0 : m_capacity() - m_bufFill;
}

void Response::beginHeaders() {
  if (m_headersSent || m_sendingHeaders || getWriteError()) return;
  if (!m_statusSent) {
    status(200);
  }

  m_sendingHeaders = true;

  bool streaming = m_responseOpen && m_statusSent >= 200;
  if (streaming) {
    m_keepAlive = false;
    m_chunkedEncoding = false;
  } else if (m_statusSent >= 200) {
    m_keepAlive = m_canKeepAlive();
    if (!m_chunkingAllowed) m_chunkedEncoding = false;
  }
  P(connectionName) = "Connection";
  P(lengthName) = "Content-Length";
  P(encodingName) = "Transfer-Encoding";
  P(headerSeparator) = ": ";
  bool connectionWritten = false;
  for (int i = 0; i < m_headersCount; i++) {
    const char* name = m_headers[i].name;
    bool length = App::strcmpiP(name, lengthName) == 0;
    bool encoding = App::strcmpiP(name, encodingName) == 0;
    bool connection = App::strcmpiP(name, connectionName) == 0;
    if (streaming && (length || encoding || connection)) continue;
    if (encoding && !m_chunkingAllowed) continue;
    if ((m_statusSent < 200 || m_statusSent == 204) && (length || encoding))
      continue;
    if (m_statusSent == 205 && (length || encoding)) continue;
    if (length && m_chunkedEncoding) continue;
    if (m_statusSent >= 200 && connection && !m_keepAlive) {
      P(closeConnection) = "close";
      if (App::strcmpiP(m_headers[i].value, closeConnection) != 0) continue;
    }
    this->print(m_headers[i].name);
    printP(headerSeparator);
    this->print(m_headers[i].value);
    m_printCRLF();
    if (connection) connectionWritten = true;
  }
  if (m_statusSent >= 200) {
    if (!m_contentTypeSet && m_statusSent != 204 && m_statusSent != 205 &&
        m_statusSent != 304) {
      P(contentType) = "Content-Type: text/plain\r\n";
      printP(contentType);
    }
    if (m_chunked() && !get("Transfer-Encoding")) {
      P(chunked) = "Transfer-Encoding: chunked\r\n";
      printP(chunked);
    }
    if (m_statusSent == 205) {
      P(emptyContent) = "Content-Length: 0\r\n";
      printP(emptyContent);
    }
    if (streaming || (!m_keepAlive && !connectionWritten)) {
      P(close) = "Connection: close\r\n";
      printP(close);
    }
  }
}

int Response::bytesSent() { return m_bytesSent; }

void Response::end() { m_ended = true; }

void Response::endHeaders() {
  if (m_headersSent || getWriteError()) return;
  if (!m_sendingHeaders) beginHeaders();
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

void Response::keepOpen() {
  if (!getWriteError() && !m_head) m_responseOpen = true;
}

void Response::printP(const unsigned char* string) {
  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  while (uint8_t value = pgm_read_byte(string++)) {
    if (!write(value)) break;
  }
}

void Response::printP(const char* string) { printP((unsigned char*)string); }

void Response::sendStatus(int code) {
  status(code);

  if (code < 200) return;
  m_printHeaders();

  if (m_bodyAllowed()) {
    m_printStatus(code);
  }
}

void Response::set(const char* name, const char* value) {
  if (m_headersSent || getWriteError()) return;
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

  P(transferEncoding) = "Transfer-Encoding";
  P(chunked) = "chunked";
  if (App::strcmpiP(name, transferEncoding) == 0 &&
      App::strcmpiP(value, chunked) == 0) {
    m_chunkedEncoding = true;
  }

  P(connection) = "Connection";
  if (App::strcmpiP(name, connection) == 0) {
    P(keepAlive) = "keep-alive";
    m_keepAlive = App::strcmpiP(value, keepAlive) == 0;
  }
}

void Response::setDefaults() {}

bool Response::m_canStartChannel(int code, int headerCount) {
  return !getWriteError() && !m_ended && !m_headersSent && !m_sendingHeaders &&
         (!m_statusSent || m_statusSent == code) &&
         headerCount <= SERVER_MAX_HEADERS - m_headersCount;
}

bool Response::m_canKeepAlive() const {
  if (!m_keepAlive || !m_requestKeepAlive || m_responseOpen) return false;
  return !m_bodyAllowed() || m_contentLengthSet || m_chunkingAllowed;
}

bool Response::m_bodyAllowed() const {
  return !m_head && m_statusSent >= 200 && m_statusSent != 204 &&
         m_statusSent != 205 && m_statusSent != 304;
}

bool Response::m_chunked() const {
  return m_chunkingAllowed && m_bodyAllowed() &&
         (m_chunkedEncoding || (m_keepAlive && !m_contentLengthSet));
}

void Response::status(int code) {
  if (m_statusSent || getWriteError()) {
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

  if (code < 200 && code != 101) {
    beginHeaders();
    endHeaders();
    m_statusSent = 0;
    m_headersSent = false;
    m_headersCount = 0;
    m_contentLengthSet = false;
    m_chunkedEncoding = false;
    m_contentTypeSet = false;
    m_keepAlive = false;
    m_responseOpen = false;
  }

  m_sendingStatus = false;
}

int Response::statusSent() { return m_statusSent; }

size_t Response::write(uint8_t data) {
  if (getWriteError()) return 0;
  if (m_shouldPrintHeaders()) m_printHeaders();
  if (getWriteError()) return 0;
  if (m_headersSent && !m_bodyAllowed()) return 1;
  m_buffer[CHUNK_HEADER_SIZE + m_bufFill++] = data;
  if (m_bufFill == m_capacity()) m_flushBuf();
  if (getWriteError()) return 0;
  if (m_bytesSent < INT_MAX) ++m_bytesSent;
  return 1;
}

size_t Response::write(const uint8_t* buffer, size_t bufferLength) {
  if (getWriteError() || bufferLength == 0) return 0;
  if (m_shouldPrintHeaders()) m_printHeaders();
  if (getWriteError()) return 0;
  if (m_headersSent && !m_bodyAllowed()) return bufferLength;

  size_t written = 0;
  int capacity = m_capacity();
  if (bufferLength < (size_t)(capacity - m_bufFill)) {
    memcpy(m_buffer + CHUNK_HEADER_SIZE + m_bufFill, buffer, bufferLength);
    m_bufFill += bufferLength;
    written = bufferLength;
  } else {
    m_flushBuf();
    while (written < bufferLength && !getWriteError()) {
      size_t length = bufferLength - written;
      bool chunked = m_headersSent && m_chunked();
      if (chunked && length > (size_t)capacity) length = capacity;
      if (chunked) {
        int headerLength = m_writeChunkHeader((int)length);
        m_writeAll(m_buffer + CHUNK_HEADER_SIZE - headerLength, headerLength);
        if (getWriteError()) break;
      }
      written += m_writeAll(buffer + written, length);
      if (chunked && !getWriteError()) {
        m_writeAll((const uint8_t*)CRLF, 2);
      }
    }
  }
  m_bytesSent += (int)min(written, (size_t)(INT_MAX - m_bytesSent));
  return written;
}

void Response::m_failWrite() {
  setWriteError();
  m_keepAlive = false;
  m_responseOpen = false;
  m_ended = true;
  m_bufFill = 0;
  m_stream->stop();
}

size_t Response::m_writeAll(const uint8_t* buffer, size_t length) {
  size_t written = 0;
  while (written < length && !getWriteError()) {
    size_t count = m_stream->write(buffer + written, length - written);
    if (count == 0 || count > length - written) {
      m_failWrite();
      break;
    }
    written += count;
  }
  return written;
}

void Response::writeP(const unsigned char* data, size_t length) {
  if (m_shouldPrintHeaders()) {
    m_printHeaders();
  }

  while (length-- && !getWriteError()) {
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
  if (m_headersSent || getWriteError()) return;
  beginHeaders();
  endHeaders();
}

void Response::m_printCRLF() { print(CRLF); }

void Response::m_flushBuf() {
  if (m_bufFill > 0 && !getWriteError()) {
    if (m_headersSent && m_chunked()) {
      int headerLen = m_writeChunkHeader(m_bufFill);

      m_buffer[CHUNK_HEADER_SIZE + m_bufFill] = '\r';
      m_buffer[CHUNK_HEADER_SIZE + m_bufFill + 1] = '\n';

      int startOffset = CHUNK_HEADER_SIZE - headerLen;
      m_writeAll(m_buffer + startOffset,
                 headerLen + m_bufFill + CHUNK_TRAILER_SIZE);
    } else {
      m_writeAll(m_buffer + CHUNK_HEADER_SIZE, m_bufFill);
    }

    m_bufFill = 0;
  }
}

void Response::m_finalize() {
  m_flushBuf();

  if (!getWriteError() && !m_responseOpen && m_headersSent && m_chunked()) {
    m_writeAll((const uint8_t*)"0\r\n\r\n", 5);
  }
}

int Response::m_writeChunkHeader(int dataSize) {
  static const char hexDigits[] = "0123456789abcdef";

  m_buffer[CHUNK_HEADER_SIZE - 2] = '\r';
  m_buffer[CHUNK_HEADER_SIZE - 1] = '\n';

  int pos = CHUNK_HEADER_SIZE - 3;
  int hexLen = 0;
  int n = dataSize;

  do {
    m_buffer[pos--] = hexDigits[n & 0xF];
    n >>= 4;
    hexLen++;
  } while (n > 0 && pos >= 0);

  return hexLen + 2;
}
