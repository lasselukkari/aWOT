/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things
*/

#ifndef AWOT_ROUTER_H_
#define AWOT_ROUTER_H_

#include "Request.h"

namespace awot {

class Router {
  friend class App;

 public:
#ifdef STD_FUNCTION_MIDDLEWARE
#define MIDDLEWARE_PARAM Middleware
#define MIDDLEWARE_FUNCTION \
  std::function<void(Request & request, Response & response)> Middleware
#else
#define MIDDLEWARE_PARAM Middleware*
#define MIDDLEWARE_FUNCTION \
  void Middleware(Request& request, Response& response)
#endif

  typedef MIDDLEWARE_FUNCTION;

  Router();
  Router(const Router&) = delete;
  Router& operator=(const Router&) = delete;
  ~Router();

  void del(const char* path, MIDDLEWARE_PARAM middleware);
  void del(MIDDLEWARE_PARAM middleware);
  void get(const char* path, MIDDLEWARE_PARAM middleware);
  void get(MIDDLEWARE_PARAM middleware);
  void head(const char* path, MIDDLEWARE_PARAM middleware);
  void head(MIDDLEWARE_PARAM middleware);
  void options(const char* path, MIDDLEWARE_PARAM middleware);
  void options(MIDDLEWARE_PARAM middleware);
  void patch(const char* path, MIDDLEWARE_PARAM middleware);
  void patch(MIDDLEWARE_PARAM middleware);
  void post(const char* path, MIDDLEWARE_PARAM middleware);
  void post(MIDDLEWARE_PARAM middleware);
  void put(const char* path, MIDDLEWARE_PARAM middleware);
  void put(MIDDLEWARE_PARAM middleware);
  void use(const char* path, Router* router);
  void use(Router* router);
  void use(const char* path, MIDDLEWARE_PARAM middleware);
  void use(MIDDLEWARE_PARAM middleware);

#ifdef STD_FUNCTION_MIDDLEWARE
  template <typename T>
  void del(const char* path, T* instance,
           void (T::*method)(Request&, Response&)) {
    del(path, std::bind(method, instance, std::placeholders::_1,
                        std::placeholders::_2));
  }
  template <typename T>
  void del(T* instance, void (T::*method)(Request&, Response&)) {
    del(std::bind(method, instance, std::placeholders::_1,
                  std::placeholders::_2));
  }
  template <typename T>
  void get(const char* path, T* instance,
           void (T::*method)(Request&, Response&)) {
    get(path, std::bind(method, instance, std::placeholders::_1,
                        std::placeholders::_2));
  }
  template <typename T>
  void get(T* instance, void (T::*method)(Request&, Response&)) {
    get(std::bind(method, instance, std::placeholders::_1,
                  std::placeholders::_2));
  }
  template <typename T>
  void head(const char* path, T* instance,
            void (T::*method)(Request&, Response&)) {
    head(path, std::bind(method, instance, std::placeholders::_1,
                         std::placeholders::_2));
  }
  template <typename T>
  void head(T* instance, void (T::*method)(Request&, Response&)) {
    head(std::bind(method, instance, std::placeholders::_1,
                   std::placeholders::_2));
  }
  template <typename T>
  void options(const char* path, T* instance,
               void (T::*method)(Request&, Response&)) {
    options(path, std::bind(method, instance, std::placeholders::_1,
                            std::placeholders::_2));
  }
  template <typename T>
  void options(T* instance, void (T::*method)(Request&, Response&)) {
    options(std::bind(method, instance, std::placeholders::_1,
                      std::placeholders::_2));
  }
  template <typename T>
  void patch(const char* path, T* instance,
             void (T::*method)(Request&, Response&)) {
    patch(path, std::bind(method, instance, std::placeholders::_1,
                          std::placeholders::_2));
  }
  template <typename T>
  void patch(T* instance, void (T::*method)(Request&, Response&)) {
    patch(std::bind(method, instance, std::placeholders::_1,
                    std::placeholders::_2));
  }
  template <typename T>
  void post(const char* path, T* instance,
            void (T::*method)(Request&, Response&)) {
    post(path, std::bind(method, instance, std::placeholders::_1,
                         std::placeholders::_2));
  }
  template <typename T>
  void post(T* instance, void (T::*method)(Request&, Response&)) {
    post(std::bind(method, instance, std::placeholders::_1,
                   std::placeholders::_2));
  }
  template <typename T>
  void put(const char* path, T* instance,
           void (T::*method)(Request&, Response&)) {
    put(path, std::bind(method, instance, std::placeholders::_1,
                        std::placeholders::_2));
  }
  template <typename T>
  void put(T* instance, void (T::*method)(Request&, Response&)) {
    put(std::bind(method, instance, std::placeholders::_1,
                  std::placeholders::_2));
  }
  template <typename T>
  void use(const char* path, T* instance,
           void (T::*method)(Request&, Response&)) {
    use(path, std::bind(method, instance, std::placeholders::_1,
                        std::placeholders::_2));
  }
  template <typename T>
  void use(T* instance, void (T::*method)(Request&, Response&)) {
    use(std::bind(method, instance, std::placeholders::_1,
                  std::placeholders::_2));
  }
#endif

 private:
  struct MiddlewareNode {
    const char* path;
    MIDDLEWARE_PARAM middleware;
    Router* router;
    Request::MethodType type;
    MiddlewareNode* next;
  };

  void m_addMiddleware(Request::MethodType type, const char* path,
                       MIDDLEWARE_PARAM middleware);
  void m_mountMiddleware(MiddlewareNode* tail);
  void m_dispatchMiddleware(Request& request, Response& response,
                            int urlShift = 0);
  bool m_routeMatch(const char* route, const char* pattern);

  MiddlewareNode* m_head;
};

}  // namespace awot

#endif
