---
title: "aWOT basic routing"
description: "Register HTTP routes and continue through multiple handlers with aWOT 4."
route: "/en/starter/basic-routing.html"
section: "starter"
---

# Basic routing

A route pairs an HTTP method and a path with a handler. An `App` named `app`
registers routes in `setup()` and processes incoming clients in `loop()`, as in
the [complete hello world sketches](/en/starter/hello-world.html).

The following are partial examples: place the handler functions above `setup()`
in a hello world sketch and add their registrations before `server.begin()`.

```cpp
void readHome(Request& req, Response& res) {
  res.print("Hello World!");
}

void createUser(Request& req, Response& res) {
  res.status(201);
  res.print("Created a user");
}

void updateUser(Request& req, Response& res) {
  res.print("Updated the user");
}

void removeUser(Request& req, Response& res) {
  res.sendStatus(204);
}
```

```cpp
// Add inside setup().
app.get("/", readHome);
app.post("/user", createUser);
app.put("/user", updateUser);
app.del("/user", removeUser);
```

Use `del()` for DELETE because `delete` is a C++ keyword. The supported route
methods are `get`, `head`, `post`, `put`, `patch`, `del`, and `options`. HEAD
needs its own registration; aWOT suppresses its response body. `app.use(handler)`
runs for every supported method and path. `app.use("/user", handler)` matches
that route pattern for every method.

<h2 id="multiple-handlers">Multiple handlers and continuation</h2>


Register the same route more than once to run several handlers in order. Each
handler must call `req.next()` if it wants dispatch to continue. This partial
example logs a request before producing the response:

```cpp
void logUser(Request& req, Response& res) {
  Serial.println(req.path());
  req.next();
}

void readUser(Request& req, Response& res) {
  res.print("User details");
}
```

```cpp
// Add inside setup().
app.get("/user", logUser);
app.get("/user", readUser);
```

Returning without `req.next()` stops routing. `res.end()` marks the response as
ended and also prevents subsequent route handlers, even if `req.next()` was
called. aWOT finalizes the output when `process()` returns; the caller then
closes or retains the client.

See [routing](/en/guide/routing.html) for parameters and mounted routers, and
[middleware](/en/guide/middleware.html) for logging and authentication.
