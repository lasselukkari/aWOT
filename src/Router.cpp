#include "Router.h"

using namespace awot;

Router::Router() : m_head(NULL) {}

Router::~Router() {
  MiddlewareNode* current = m_head;
  MiddlewareNode* next;

  while (current != NULL) {
    next = current->next;
    delete current;

    current = next;
  }

  m_head = NULL;
}

void Router::del(const char* path, MIDDLEWARE_PARAM middleware) {
  m_addMiddleware(Request::DELETE, path, middleware);
}

void Router::del(MIDDLEWARE_PARAM middleware) { del(NULL, middleware); }

void Router::get(const char* path, MIDDLEWARE_PARAM middleware) {
  m_addMiddleware(Request::GET, path, middleware);
}

void Router::get(MIDDLEWARE_PARAM middleware) { get(NULL, middleware); }

void Router::head(const char* path, MIDDLEWARE_PARAM middleware) {
  m_addMiddleware(Request::HEAD, path, middleware);
}

void Router::head(MIDDLEWARE_PARAM middleware) { head(NULL, middleware); }

void Router::options(const char* path, MIDDLEWARE_PARAM middleware) {
  m_addMiddleware(Request::OPTIONS, path, middleware);
}

void Router::options(MIDDLEWARE_PARAM middleware) { options(NULL, middleware); }

void Router::post(const char* path, MIDDLEWARE_PARAM middleware) {
  m_addMiddleware(Request::POST, path, middleware);
}

void Router::post(MIDDLEWARE_PARAM middleware) { post(NULL, middleware); }

void Router::put(const char* path, MIDDLEWARE_PARAM middleware) {
  m_addMiddleware(Request::PUT, path, middleware);
}

void Router::put(MIDDLEWARE_PARAM middleware) { put(NULL, middleware); }

void Router::patch(const char* path, MIDDLEWARE_PARAM middleware) {
  m_addMiddleware(Request::PATCH, path, middleware);
}

void Router::patch(MIDDLEWARE_PARAM middleware) { patch(NULL, middleware); }

void Router::use(const char* path, MIDDLEWARE_PARAM middleware) {
  m_addMiddleware(Request::ALL, path, middleware);
}

void Router::use(MIDDLEWARE_PARAM middleware) { use(NULL, middleware); }

void Router::use(const char* path, Router* router) {
  MiddlewareNode* tail = new MiddlewareNode();
  if (!tail) return;
  tail->path = path;
  tail->middleware = NULL;
  tail->router = router;
  tail->next = NULL;
  m_mountMiddleware(tail);
}

void Router::use(Router* router) { use(NULL, router); }

void Router::m_addMiddleware(Request::MethodType type, const char* path,
                             MIDDLEWARE_PARAM middleware) {
  MiddlewareNode* tail = new MiddlewareNode();
  if (!tail) return;
  tail->path = path;
  tail->middleware = middleware;
  tail->router = NULL;
  tail->type = type;
  tail->next = NULL;

  m_mountMiddleware(tail);
}

void Router::m_mountMiddleware(MiddlewareNode* tail) {
  if (m_head == NULL) {
    m_head = tail;
  } else {
    MiddlewareNode* current = m_head;

    while (current->next != NULL) {
      current = current->next;
    }

    current->next = tail;
  }
}

void Router::m_dispatchMiddleware(Request& request, Response& response,
                                  int urlShift) {
  MiddlewareNode* middleware = m_head;

  while (middleware != NULL && !response.ended() && request.m_next) {
    if (middleware->router != NULL) {
      int prefixLength = middleware->path ? strlen(middleware->path) : 0;
      while (prefixLength > 0 && middleware->path[prefixLength - 1] == '/') {
        --prefixLength;
      }
      int shift = urlShift + prefixLength;

      const char* pathAtShift = request.path() + urlShift;
      if (middleware->path == NULL ||
          (strncmp(middleware->path, pathAtShift, prefixLength) == 0 &&
           (pathAtShift[prefixLength] == '/' ||
            pathAtShift[prefixLength] == '\0'))) {
        middleware->router->m_dispatchMiddleware(request, response, shift);
      }
    } else if (middleware->type == request.method() ||
               middleware->type == Request::ALL) {
      if (middleware->path == NULL ||
          m_routeMatch(request.path() + urlShift, middleware->path)) {
        request.m_setRoute(request.path() + urlShift, middleware->path);
        request.m_next = false;
        middleware->middleware(request, response);
      }
    }

    middleware = middleware->next;
  }
}

bool Router::m_routeMatch(const char* route, const char* pattern) {
  if (route[0] == '\0') {
    return pattern[0] == '\0' || (pattern[0] == '/' && pattern[1] == '\0');
  }
  if (pattern[0] == '\0') {
    return route[0] == '/' && route[1] == '\0';
  }

  bool match = false;
  int i = 0;
  int j = 0;

  while (pattern[i] && route[j]) {
    if (pattern[i] == ':') {
      while (pattern[i] && pattern[i] != '/') {
        i++;
      }

      while (route[j] && route[j] != '/') {
        j++;
      }

      match = true;
    } else if (pattern[i] == route[j]) {
      j++;
      i++;
      match = true;
    } else {
      match = false;
      break;
    }
  }

  if (match && !pattern[i] && route[j] == '/' && !route[j + 1]) {
    match = true;
  } else if (pattern[i] || route[j]) {
    match = false;
  }

  return match;
}
