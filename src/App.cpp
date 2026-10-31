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

App::App() : m_final(), m_notFound(), m_errorHandler(NULL), m_timeout(1000) {}

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

  if (response.statusSent() && !response.headersSent()) {
    response.m_printHeaders();
  }

  if (m_final) {
    m_final(request, response);
  }

  int remaining = request.left();
  bool parsed = request.m_readingContent && !request.m_bodyError();
  bool bodyComplete = remaining == 0;
  response.m_finalize();

  result.responseOpen =
      parsed && !response.getWriteError() && response.m_responseOpen &&
      (request.m_bodyState == Request::CONTENT_LENGTH || bodyComplete);
  result.keepAlive = parsed && bodyComplete && !response.getWriteError() &&
                     !response.m_responseOpen && response.m_keepAlive &&
                     request.m_allowsKeepAlive();
  if (!parsed) client->stop();

  for (int i = 0; i < headers.size(); ++i) {
    headers[i].buffer[0] = '\0';
    headers[i].m_present = false;
  }

  return result;
}

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

void App::m_sendError(Request& request, Response& response, int code) {
  if (m_errorHandler) m_errorHandler(request, response, code);
  if (!response.ended()) response.sendStatus(code);
}

void App::m_process(Request& request, Response& response) {
  if (!request.m_processMethod()) {
    return m_sendError(request, response, request.m_timedout() ? 408 : 400);
  }

  response.m_head = request.method() == Request::HEAD;

  if (!request.m_readURL()) {
    return m_sendError(request, response, request.m_timedout() ? 408 : 414);
  }

  if (!request.m_processURL()) {
    return m_sendError(request, response, 400);
  }

  if (!request.m_readVersion()) {
    return m_sendError(request, response, request.m_timedout() ? 408 : 505);
  }

  response.m_chunkingAllowed = request.minorVersion() == 1;

  int headerError = request.m_processHeaders();
  if (headerError) {
    return m_sendError(request, response,
                       request.m_timedout() ? 408 : headerError);
  }

  response.m_requestKeepAlive = request.m_allowsKeepAlive();

  m_defaultRouter.m_dispatchMiddleware(request, response);

  request.left();
  int bodyError = request.m_bodyError();
  if (bodyError) {
    response.m_requestKeepAlive = false;
    if (!response.statusSent()) {
      m_sendError(request, response, bodyError);
    }
    return;
  }

  if (!response.statusSent() && !response.ended()) {
    if (m_notFound) {
      response.status(404);
      return m_notFound(request, response);
    }

    return m_sendError(request, response, 404);
  }

  if (!response.headersSent()) {
    response.m_printHeaders();
  }
}
