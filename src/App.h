/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things
*/

#ifndef AWOT_APP_H_
#define AWOT_APP_H_

#include "Router.h"
#include "StreamClient.h"

namespace awot {

class App {
 public:
  App();
  ~App();

  static int strcmpi(const char* s1, const char* s2);
  static int strcmpiP(const char* s1, const unsigned char* s2);

  void del(const char* path, Router::MIDDLEWARE_PARAM middleware);
  void del(Router::MIDDLEWARE_PARAM middleware);
  void finally(Router::MIDDLEWARE_PARAM middleware);
  void get(const char* path, Router::MIDDLEWARE_PARAM middleware);
  void get(Router::MIDDLEWARE_PARAM middleware);
  void head(const char* path, Router::MIDDLEWARE_PARAM middleware);
  void head(Router::MIDDLEWARE_PARAM middleware);
  void notFound(Router::MIDDLEWARE_PARAM middleware);

  typedef void (*ErrorHandler)(Request&, Response&, int statusCode);
  void onError(ErrorHandler handler);

  void options(const char* path, Router::MIDDLEWARE_PARAM middleware);
  void options(Router::MIDDLEWARE_PARAM middleware);
  void patch(const char* path, Router::MIDDLEWARE_PARAM middleware);
  void patch(Router::MIDDLEWARE_PARAM middleware);
  void post(const char* path, Router::MIDDLEWARE_PARAM middleware);
  void post(Router::MIDDLEWARE_PARAM middleware);
  void put(const char* path, Router::MIDDLEWARE_PARAM middleware);
  void put(Router::MIDDLEWARE_PARAM middleware);

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
  void finally(T* instance, void (T::*method)(Request&, Response&)) {
    finally(std::bind(method, instance, std::placeholders::_1,
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
  void notFound(T* instance, void (T::*method)(Request&, Response&)) {
    notFound(std::bind(method, instance, std::placeholders::_1,
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
#endif

  struct ProcessOptions {
    char* urlBuffer = nullptr;
    int urlBufferLength = 0;
    uint8_t* writeBuffer = nullptr;
    int writeBufferLength = 0;
    Request::HeaderNode* headers = nullptr;
    int headerCount = 0;
    void* context = nullptr;
  };

  struct ProcessResult {
    bool responseOpen = false;
  };

  ProcessResult process(Client* client);
  ProcessResult process(Client* client, const ProcessOptions& options);
  ProcessResult process(Stream* stream);
  ProcessResult process(Stream* stream, const ProcessOptions& options);

  void setTimeout(unsigned long timeoutMillis);
  void use(const char* path, Router* router);
  void use(Router* router);
  void use(const char* path, Router::MIDDLEWARE_PARAM middleware);
  void use(Router::MIDDLEWARE_PARAM middleware);

#ifdef STD_FUNCTION_MIDDLEWARE
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
  void m_process(Request& req, Response& res);

  Router::MIDDLEWARE_PARAM m_final;
  Router::MIDDLEWARE_PARAM m_notFound;
  ErrorHandler m_errorHandler;
  Router m_defaultRouter;
  unsigned long m_timeout;
};

}  // namespace awot

#endif
