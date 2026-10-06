<h2 id="router">Router</h2>

`Router` groups handlers for mounting in an App or another Router. Registration
and matching follow the same rules as App routes. Include `<aWOT.h>`.

<h3 id="router-Router">Construction and ownership</h3>

```cpp
Router();
Router(const Router&) = delete;
Router& operator=(const Router&) = delete;
~Router();
```

Construct an empty Router. It owns its registration nodes and releases them on
destruction. It borrows mounted routers, path strings, and member-callback
instances; those objects must remain alive while registrations can run. A Router
cannot be copied or assigned. Mount with `app.use("/prefix", &router)`; the old
path-taking constructor is unavailable. Registration methods return `void` and
provide no allocation-failure result. Register routes once, usually in `setup()`.

<span id="router-types"></span>
<h3 id="router-Middleware">Router::Middleware</h3>

The callback type depends on the configuration in every translation unit:

```cpp
#ifdef STD_FUNCTION_MIDDLEWARE
typedef std::function<void(Request& request, Response& response)> Middleware;
#define MIDDLEWARE_PARAM Middleware
#else
typedef void Middleware(Request& request, Response& response);
#define MIDDLEWARE_PARAM Middleware*
#endif
```

The headers use `MIDDLEWARE_PARAM` for the callback parameters shown below.
Selection is automatic on ESP8266, ESP32, or platforms with `<functional>`;
`STD_FUNCTION_MIDDLEWARE` also enables it explicitly. `std::function` supports
capturing lambdas and the member overloads. Function-pointer builds support free
functions and noncapturing lambdas.

For a function-pointer build, apply `AWOT_NO_STD_FUNCTION_MIDDLEWARE` to every
library and sketch translation unit. Defining it only inside a sketch cannot
change compiled library configuration. Defining it together with
`STD_FUNCTION_MIDDLEWARE` is an error. See the
[configuration guide](/en/guide/configuration.html).

A matched callback starts with continuation disabled. `req.next()` enables the
next matching registration; returning alone stops dispatch. `res.end()` stops
dispatch even if `req.next()` was called. Router dispatch never reads or drains
the request body for the callback.

```arduino
void logRequest(Request& req, Response& res) {
  Serial.println(req.path());
  req.next();
}

void showItem(Request& req, Response& res) {
  char id[16];
  if (!req.route("id", id, sizeof(id))) {
    res.sendStatus(400);
    return;
  }
  res.print(id);
}

Router api;
// Register once:
// api.use(logRequest);
// api.get("/items/:id", showItem);
// app.use("/api", &api);
```

<span id="router-methods"></span>
<h3 id="router-get">Router::get</h3>

```cpp
void get(const char* path, MIDDLEWARE_PARAM middleware);
void get(MIDDLEWARE_PARAM middleware);
```

Register GET middleware with a route pattern, or for every GET path when the
path is omitted.

<h3 id="router-head">Router::head</h3>

```cpp
void head(const char* path, MIDDLEWARE_PARAM middleware);
void head(MIDDLEWARE_PARAM middleware);
```

Register HEAD middleware. HEAD never falls back to GET. Response bodies and
chunk terminators are suppressed for HEAD, while status and headers are sent.

<h3 id="router-post">Router::post</h3>

```cpp
void post(const char* path, MIDDLEWARE_PARAM middleware);
void post(MIDDLEWARE_PARAM middleware);
```

Register POST middleware, optionally restricted to a route pattern.

<h3 id="router-put">Router::put</h3>

```cpp
void put(const char* path, MIDDLEWARE_PARAM middleware);
void put(MIDDLEWARE_PARAM middleware);
```

Register PUT middleware, optionally restricted to a route pattern.

<h3 id="router-del">Router::del</h3>

```cpp
void del(const char* path, MIDDLEWARE_PARAM middleware);
void del(MIDDLEWARE_PARAM middleware);
```

Register DELETE middleware. The C++ method is named `del`.

<h3 id="router-patch">Router::patch</h3>

```cpp
void patch(const char* path, MIDDLEWARE_PARAM middleware);
void patch(MIDDLEWARE_PARAM middleware);
```

Register PATCH middleware, optionally restricted to a route pattern.

<h3 id="router-options">Router::options</h3>

```cpp
void options(const char* path, MIDDLEWARE_PARAM middleware);
void options(MIDDLEWARE_PARAM middleware);
```

Register OPTIONS middleware. Generate any required CORS headers in the handler.

<h3 id="router-use">Router::use and matching</h3>

```cpp
void use(const char* path, Router* router);
void use(Router* router);
void use(const char* path, MIDDLEWARE_PARAM middleware);
void use(MIDDLEWARE_PARAM middleware);
```

The callback overload matches every supported HTTP method. A provided callback
path matches the full route, using the same patterns as method-specific
registrations. Patterns are case-sensitive; `:name` consumes a segment and is
read with `req.route("name", buffer, capacity)`. Matching uses the decoded path,
and excludes the query. A route `/items/:id` matches `/items/42` and
`/items/42/`; a single extra trailing slash is accepted after a complete
pattern. A pattern ending in `/` still requires that slash. There are no wildcard
or regular-expression patterns. The pathless overload matches every path.

The Router overload mounts at a segment boundary. Trailing slashes on the mount
prefix are ignored, so `/api`, `/api/`, and `/api///` mount equivalently.
`/api` matches `/api`, `/api/`, and `/api/items`, but excludes `/api2/items`.
The mounted router matches its paths against the suffix after that prefix. Its
`"/"` or `""` root route matches either `/api` or `/api/`. Mounting at `/` or
without a path leaves the full path available.

Nested mounts accumulate their prefixes. For example, a child `/items/:id`
route mounted under `/v1`, inside a Router mounted under `/api`, handles
`/api/v1/items/42`. `req.path()` still returns the full decoded path, while
`req.route()` refers to the current handler's suffix and pattern. Prefixes are
literal strings and do not define route parameters.

Mounted router dispatch shares continuation state with its parent. If a child
callback returns without `req.next()`, parent dispatch stops too. If no child
route matches, or the last child callback calls `req.next()`, subsequent parent
registrations can run. The mount borrows the router rather than copying it.

<h3 id="router-memberCallbacks">Member callbacks</h3>

These overloads exist only with `STD_FUNCTION_MIDDLEWARE`:

```cpp
template <typename T>
void get(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void get(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void head(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void head(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void post(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void post(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void put(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void put(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void del(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void del(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void patch(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void patch(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void options(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void options(T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void use(const char* path, T* instance, void (T::*method)(Request&, Response&));
template <typename T>
void use(T* instance, void (T::*method)(Request&, Response&));
```

Pass an instance pointer and a non-const member function. The binding retains
the pointer and the instance must outlive dispatch. For example:

```arduino
class Controller {
 public:
  void index(Request& req, Response& res) { res.print("Controller"); }
};

Controller controller;
Router routes;
// Register once: routes.get("/", &controller, &Controller::index);
```
