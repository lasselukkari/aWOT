#include "App.h"

using namespace awot;

int App::strcmpi(const char* s1, const char* s2) {
  int i;

  for (i = 0; s1[i] && s2[i]; ++i) {
    if (tolower((unsigned char)s1[i]) != tolower((unsigned char)s2[i])) {
      break;
    }
  }

  int a = tolower((unsigned char)s1[i]);
  int b = tolower((unsigned char)s2[i]);

  if (a == b) return 0;
  return a < b ? -1 : 1;
}

int App::strcmpiP(const char* s1, const unsigned char* s2) {
  int i = 0;

  for (i = 0; s1[i] && pgm_read_byte(s2 + i); ++i) {
    if (tolower((unsigned char)s1[i]) != tolower(pgm_read_byte(s2 + i))) {
      break;
    }
  }

  int a = tolower((unsigned char)s1[i]);
  int b = tolower(pgm_read_byte(s2 + i));

  if (a == b) return 0;
  return a < b ? -1 : 1;
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

  // A null pointer selects its default capacity regardless of the unused
  // length. Supplied storage always requires a valid capacity.
  if ((options.urlBuffer.data() && options.urlBuffer.size() < 2) ||
      (options.writeBuffer.data() &&
       options.writeBuffer.size() <= CHUNK_OVERHEAD) ||
      options.headers.size() < 0 ||
      (options.headers.size() > 0 && !options.headers.data())) {
    client->stop();
    return result;
  }
  for (int i = 0; i < options.headers.size(); ++i) {
    const Request::Header& header = options.headers[i];
    if (!header.name || !header.buffer.data() || header.buffer.size() <= 0) {
      client->stop();
      return result;
    }
  }
  if (!options.urlBuffer.data())
    return m_processWithDefaultUrl(client, options);
  if (!options.writeBuffer.data())
    return m_processWithDefaultWrite(client, options);

  ArrayView<Request::Header> headers = options.headers;
  for (int i = 0; i < headers.size(); ++i) {
    headers[i].buffer[0] = '\0';
    headers[i].m_present = false;
  }

  Response response(client, options.writeBuffer.data(),
                    options.writeBuffer.size());
  Request request(client, &response, headers, options.urlBuffer.data(),
                  options.urlBuffer.size(), m_timeout, options.context);

  m_process(request, response);

  if (m_final != NULL) {
    m_final(request, response);
  }

  response.m_finalize();

  bool parsed = request.m_readingContent;
  result.responseOpen =
      parsed && !response.getWriteError() && response.m_responseOpen;
  result.keepAlive = parsed && request.left() == 0 &&
                     !response.getWriteError() && !response.m_responseOpen &&
                     response.m_keepAlive;
  if (!parsed) client->stop();

  for (int i = 0; i < headers.size(); ++i) {
    headers[i].buffer[0] = '\0';
    headers[i].m_present = false;
  }

  return result;
}

// Keep convenience storage out of the stack frame used with caller buffers.
// Both helpers are noinline so optimization cannot fold these arrays back in.
App::ProcessResult App::m_processWithDefaultUrl(Client* client,
                                                ProcessOptions options) {
  char buffer[SERVER_URL_BUFFER_SIZE];
  options.urlBuffer = buffer;
  return process(client, options);
}

App::ProcessResult App::m_processWithDefaultWrite(Client* client,
                                                  ProcessOptions options) {
  uint8_t buffer[SERVER_OUTPUT_BUFFER_SIZE];
  options.writeBuffer = buffer;
  return process(client, options);
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

  response.m_head = request.method() == Request::HEAD;

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

  if (!request.m_processURL()) {
    if (m_errorHandler) m_errorHandler(request, response, 400);
    if (!response.ended()) return response.sendStatus(400);
    return;
  }

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

  int headerError = request.m_processHeaders();
  if (headerError) {
    if (request.m_timedout()) {
      if (m_errorHandler) m_errorHandler(request, response, 408);
      if (!response.ended()) return response.sendStatus(408);
      return;
    }

    if (m_errorHandler) m_errorHandler(request, response, headerError);
    if (!response.ended()) return response.sendStatus(headerError);
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
