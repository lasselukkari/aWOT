/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things
*/

#include "App.h"

using namespace awot;

int App::strcmpi(const char* s1, const char* s2) {
  int i;

  for (i = 0; s1[i] && s2[i]; ++i) {
    if (s1[i] == s2[i] || (s1[i] ^ 32) == s2[i]) {
      continue;
    } else {
      break;
    }
  }

  if (s1[i] == s2[i]) {
    return 0;
  }

  if ((s1[i] | 32) < (s2[i] | 32)) {
    return -1;
  }

  return 1;
}

int App::strcmpiP(const char* s1, const unsigned char* s2) {
  int i = 0;

  for (i = 0; s1[i] && pgm_read_byte(s2 + i); ++i) {
    if (s1[i] == pgm_read_byte(s2 + i) ||
        (s1[i] ^ 32) == pgm_read_byte(s2 + i)) {
      continue;
    } else {
      break;
    }
  }

  if (s1[i] == pgm_read_byte(s2 + i)) {
    return 0;
  }

  if ((s1[i] | 32) < (pgm_read_byte(s2 + i) | 32)) {
    return -1;
  }

  return 1;
}

App::App()
    : m_final(NULL), m_notFound(NULL), m_errorHandler(NULL), m_timeout(1000) {}

App::~App() {}

void App::onError(ErrorHandler handler) { m_errorHandler = handler; }

App::ProcessResult App::process(Client* client) {
  ProcessOptions options;
  return process(client, options);
}

App::ProcessResult App::process(Client* client, const ProcessOptions& options) {
  ProcessResult result;

  if (!client) {
    return result;
  }

  char defaultUrlBuffer[SERVER_URL_BUFFER_SIZE];
  uint8_t defaultWriteBuffer[SERVER_OUTPUT_BUFFER_SIZE];

  char* urlBuffer = options.urlBuffer ? options.urlBuffer : defaultUrlBuffer;
  int urlBufferLength = options.urlBufferLength ? options.urlBufferLength
                                                : SERVER_URL_BUFFER_SIZE;
  uint8_t* writeBuffer =
      options.writeBuffer ? options.writeBuffer : defaultWriteBuffer;
  int writeBufferLength = options.writeBufferLength ? options.writeBufferLength
                                                    : SERVER_OUTPUT_BUFFER_SIZE;

  Request::HeaderNode* headers = options.headers;
  int headerCount = options.headerCount;

  if (headerCount > 0 && headers != nullptr) {
    for (int i = 0; i < headerCount - 1; i++) {
      headers[i].next = &headers[i + 1];
    }
    headers[headerCount - 1].next = NULL;
  }

  Response response(client, writeBuffer, writeBufferLength);
  Request request(client, &response, headerCount > 0 ? headers : nullptr,
                  urlBuffer, urlBufferLength, m_timeout, options.context);

  m_process(request, response);

  if (m_final != NULL) {
    m_final(request, response);
  }

  response.m_finalize();

  result.responseOpen = response.m_responseOpen;

  Request::HeaderNode* headerNode = headers;
  while (headerNode != NULL) {
    headerNode->buffer[0] = '\0';
    headerNode = headerNode->next;
  }

  return result;
}

App::ProcessResult App::process(Stream* stream) {
  ProcessOptions options;
  return process(stream, options);
}

App::ProcessResult App::process(Stream* stream, const ProcessOptions& options) {
  ProcessResult result;

  if (!stream) {
    return result;
  }

  StreamClient client(stream);
  return process(&client, options);
}

void App::del(const char* path, Router::MIDDLEWARE_PARAM middleware) {
  m_defaultRouter.m_addMiddleware(Request::DELETE, path, middleware);
}

void App::del(Router::MIDDLEWARE_PARAM middleware) { del(NULL, middleware); }

void App::finally(Router::MIDDLEWARE_PARAM final) { m_final = final; }

void App::get(const char* path, Router::MIDDLEWARE_PARAM middleware) {
  m_defaultRouter.m_addMiddleware(Request::GET, path, middleware);
}

void App::get(Router::MIDDLEWARE_PARAM middleware) { get(NULL, middleware); }

void App::head(const char* path, Router::MIDDLEWARE_PARAM middleware) {
  m_defaultRouter.m_addMiddleware(Request::HEAD, path, middleware);
}

void App::head(Router::MIDDLEWARE_PARAM middleware) { head(NULL, middleware); }

void App::notFound(Router::MIDDLEWARE_PARAM notFound) { m_notFound = notFound; }

void App::options(const char* path, Router::MIDDLEWARE_PARAM middleware) {
  m_defaultRouter.m_addMiddleware(Request::OPTIONS, path, middleware);
}

void App::options(Router::MIDDLEWARE_PARAM middleware) {
  options(NULL, middleware);
}

void App::patch(const char* path, Router::MIDDLEWARE_PARAM middleware) {
  m_defaultRouter.m_addMiddleware(Request::PATCH, path, middleware);
}

void App::patch(Router::MIDDLEWARE_PARAM middleware) {
  patch(NULL, middleware);
}

void App::post(const char* path, Router::MIDDLEWARE_PARAM middleware) {
  m_defaultRouter.m_addMiddleware(Request::POST, path, middleware);
}

void App::post(Router::MIDDLEWARE_PARAM middleware) { post(NULL, middleware); }

void App::put(const char* path, Router::MIDDLEWARE_PARAM middleware) {
  m_defaultRouter.m_addMiddleware(Request::PUT, path, middleware);
}

void App::put(Router::MIDDLEWARE_PARAM middleware) { put(NULL, middleware); }

void App::use(const char* path, Router::MIDDLEWARE_PARAM middleware) {
  m_defaultRouter.m_addMiddleware(Request::ALL, path, middleware);
}

void App::use(Router::MIDDLEWARE_PARAM middleware) { use(NULL, middleware); }

void App::setTimeout(unsigned long timeoutMillis) { m_timeout = timeoutMillis; }

void App::use(const char* path, Router* router) {
  m_defaultRouter.use(path, router);
}

void App::use(Router* router) { use(NULL, router); }

void App::m_process(Request& request, Response& response) {
  if (!request.m_processMethod()) {
    if (request.m_timedout()) {
      if (m_errorHandler) m_errorHandler(request, response, 408);
      if (!response.ended()) return response.sendStatus(408);
      return;
    }

    if (m_errorHandler) m_errorHandler(request, response, 400);
    if (!response.ended()) return response.sendStatus(400);
    return;
  }

  if (!request.m_readURL()) {
    if (request.m_timedout()) {
      if (m_errorHandler) m_errorHandler(request, response, 408);
      if (!response.ended()) return response.sendStatus(408);
      return;
    }

    if (m_errorHandler) m_errorHandler(request, response, 414);
    if (!response.ended()) return response.sendStatus(414);
    return;
  }

  request.m_processURL();

  if (!request.m_readVersion()) {
    if (request.m_timedout()) {
      if (m_errorHandler) m_errorHandler(request, response, 408);
      if (!response.ended()) return response.sendStatus(408);
      return;
    }

    if (m_errorHandler) m_errorHandler(request, response, 505);
    if (!response.ended()) return response.sendStatus(505);
    return;
  }

  if (!request.m_processHeaders()) {
    if (request.m_timedout()) {
      if (m_errorHandler) m_errorHandler(request, response, 408);
      if (!response.ended()) return response.sendStatus(408);
      return;
    }

    if (m_errorHandler) m_errorHandler(request, response, 431);
    if (!response.ended()) return response.sendStatus(431);
    return;
  }

  m_defaultRouter.m_dispatchMiddleware(request, response);

  if (!response.statusSent() && !response.ended()) {
    if (m_notFound != NULL) {
      response.status(404);
      return m_notFound(request, response);
    }

    if (m_errorHandler) m_errorHandler(request, response, 404);
    if (!response.ended()) return response.sendStatus(404);
    return;
  }

  if (!response.headersSent()) {
    response.m_printHeaders();
  }
}
