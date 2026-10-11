---
title: "aWOT routing"
description: "Match HTTP methods and paths, read route parameters, and mount routers in aWOT."
route: "/en/guide/routing.html"
section: "guide"
---

# Routing

An `App` selects handlers using an HTTP method and the decoded URL path.
Registrations run in order. Call `req.next()` to continue to the next matching
handler; returning without it stops dispatch. See
[basic routing](/en/starter/basic-routing.html) and
[middleware](/en/guide/middleware.html).

The snippets below are partial examples for the
[hello world sketch](/en/starter/hello-world.html): declare handlers and routers
at file scope and put registrations in `setup()` before `server.begin()`.

<h2 id="route-methods">Route methods</h2>


Use `get`, `head`, `post`, `put`, `patch`, `del`, and `options` for the
corresponding methods. There is no `app.all()` method: use `app.use()` for all
supported methods. A registration without a path matches every path for that
method. HEAD requires an explicit `head()` route and suppresses payload output.

```cpp
void readIndex(Request& req, Response& res) {
  res.print("GET request to the homepage");
}

void createItem(Request& req, Response& res) {
  res.status(201);
  res.print("Created an item");
}

void headIndex(Request& req, Response& res) {
  res.set("Content-Type", "text/plain");
  res.set("Content-Length", "27");
  res.status(200);
}
```

```cpp
// Add inside setup().
app.get("/", readIndex);
app.post("/", createItem);
app.head("/", headIndex);
```

<h2 id="route-paths">Route paths</h2>


Paths are case sensitive. Query strings do not participate in route matching:
`/about?view=short` matches `/about`. `req.path()` returns a percent-decoded
path; `req.query()` returns the raw query without `?`. A literal `+` remains
`+` in the path. See [request data](/en/guide/request-data.html) for decoding.

Callback registrations use a complete route pattern. For example,
`app.use("/secret", handler)` matches `/secret` and `/secret/`, but does not
match `/secret/page`. `app.use(handler)` has no path restriction. A single
trailing slash on the request is accepted for a pattern without one:
`/about` matches `/about/`. A pattern ending in `/` requires that slash.
The root patterns `""` and `"/"` also match the root of a mounted router.

Patterns support `:name` parameters for path segments. They do not implement
Express regular expressions, wildcards, or optional segments.

<h3 id="route-parameters">Route parameters</h3>


For `/users/34/books/8989`, this partial handler extracts both named segments.
The capacity includes the NUL terminator; always check `req.route()` before
using the result.

```cpp
void usersBook(Request& req, Response& res) {
  char user[10];
  char book[10];
  if (!req.route("userId", user, sizeof(user)) ||
      !req.route("bookId", book, sizeof(book))) {
    res.sendStatus(400);
    return;
  }
  res.print("User: ");
  res.println(user);
  res.print("Book: ");
  res.println(book);
}
```

```cpp
// Add inside setup().
app.get("/users/:userId/books/:bookId", usersBook);
```

Route values come from the decoded path. Encoded slashes become segment
separators during decoding; they cannot hide a slash inside a parameter.

<h2 id="aWOT-router">Router</h2>


`Router` groups registrations. Mount a router with `app.use("/prefix", &router)`.
Mount matching uses a literal prefix at a path segment boundary, rather than a
callback route pattern. `/cats` matches `/cats`, `/cats/`, and `/cats/long`, but
not `/catsup`. Trailing slashes on a mount are ignored. Mounts do not capture
`:name` parameters.

```cpp
Router cats;
Router details;

void catIndex(Request& req, Response& res) {
  res.print("Try /cats/long or /cats/details/nyan");
}

void longCat(Request& req, Response& res) {
  res.set("Content-Type", "text/html");
  res.print("<p>looooong cat is long!</p>");
}

void catDetails(Request& req, Response& res) {
  char name[32];
  if (!req.route("name", name, sizeof(name))) {
    res.sendStatus(400);
    return;
  }
  res.print("Details for ");
  res.print(name);
}
```

```cpp
// Add inside setup().
cats.get("/", catIndex);
cats.get("/long", longCat);
details.get("/:name", catDetails);
cats.use("/details", &details);
app.use("/cats", &cats);
```

Nested mounts accumulate prefixes: `catDetails` handles
`/cats/details/nyan`. `req.path()` still exposes the complete decoded path;
`req.route()` uses the pattern inside the currently executing router. A mounted
handler that calls `req.next()` allows matching registrations after the mount
to run too.

Apps and routers cannot be copied. They own their registration nodes but
borrow mounted routers and registered path strings. Keep routers and path
strings alive while requests may use them; file-scope declarations and string
literals satisfy that requirement.

<h2 id="member-callbacks">Member callbacks and lambdas</h2>


On platforms with automatic `std::function` support, including ESP32, aWOT
accepts member callbacks and capturing lambdas. This partial example belongs
at file scope, with its registration in `setup()`:

```cpp
class Counter {
 public:
  void handle(Request& req, Response& res) {
    res.print(++count);
  }
 private:
  unsigned long count = 0;
};

Counter counter;
```

```cpp
// Add inside setup() on a std::function build.
app.get("/count", &counter, &Counter::handle);
```

Keep the instance alive as long as its registration. Function-pointer builds
accept ordinary functions and noncapturing lambdas. See
[configuration](/en/guide/configuration.html) before changing callback macros.
