/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things
*/

#ifndef AWOT_RESPONSE_H_
#define AWOT_RESPONSE_H_

#include "aWOT_common.h"

namespace awot {

class Response : public Print {
  friend class App;
  friend class Router;

 public:
  int availableForWrite();
  int bytesSent();
  void beginHeaders();
  void end();
  void endHeaders();
  bool ended();
  void flush();
  const char* get(const char* name);
  bool headersSent();
  void printP(const unsigned char* string);
  void printP(const char* string);
  void sendStatus(int code);
  void set(const char* name, const char* value);
  void setDefaults();
  void status(int code);
  int statusSent();
  size_t write(uint8_t data);
  size_t write(const uint8_t* buffer, size_t bufferLength);
  void writeP(const unsigned char* data, size_t length);
  void keepOpen();

 private:
  Response(Client* client, uint8_t* writeBuffer, int writeBufferLength);

  void m_printStatus(int code);
  bool m_shouldPrintHeaders();
  void m_printHeaders();
  void m_printCRLF();
  void m_flushBuf();
  void m_finalize();
  int m_writeChunkHeader(int dataSize);

  Client* m_stream;
  struct Headers {
    const char* name;
    const char* value;
  } m_headers[SERVER_MAX_HEADERS];
  bool m_contentLengthSet;
  bool m_contentTypeSet;
  bool m_keepAlive;
  bool m_responseOpen;
  int m_statusSent;
  bool m_headersSent;
  bool m_sendingStatus;
  bool m_sendingHeaders;
  int m_headersCount;
  int m_bytesSent;
  bool m_ended;
  uint8_t* m_buffer;
  int m_bufferLength;
  int m_bufFill;
};

}  // namespace awot

#endif
