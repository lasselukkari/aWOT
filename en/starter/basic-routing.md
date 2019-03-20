---
layout: page
title: aWOT basic routing
menu: starter
lang: en
redirect_from: "/starter/basic-routing.html"
---

# Basic routing

_Routing_ refers to determining how an application responds to a client request to a particular endpoint, which is a URI (or path) and a specific HTTP request method (GET, POST, and so on).

Each route can have one or more handler functions, which are executed when the route is matched.

Route definition takes the following structure:

```arduino
app.METHOD(PATH, HANDLER)
```

Where:

- `app` is an instance of `aWOT Application`.
- `METHOD` is an [HTTP request method](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol#Request_methods), in lowercase.
- `PATH` is a path on the server.
- `HANDLER` is the Middleware executed when the route is matched.

<div class="doc-box doc-notice" markdown="1">
This tutorial assumes that a network connection has been created and instance of `aWOT Application` named `app` is processing incoming connections in the main loop. If you are not familiar with creating an app and starting it, see the [Hello world example](/{{ page.lang }}/starter/hello-world.html).
</div>

The following examples illustrate defining simple routes.

Respond with `Hello World!` on the homepage:

```arduino
void index(Request &req, Response &res) {
  res.print("Hello World!");
}

void setup() {
  app.get("/", &index);
}
```

Respond to POST request on the root route (`/`), the application's home page:

```arduino
void post(Request &req, Response &res) {
  res.print("Got a POST request");
}

void setup() {
  app.post("/", &post);
}
```

Respond to a PUT request to the `/user` route:

```arduino
void updateUser(Request &req, Response &res) {
  res.print("Got a PUT request at /user");
}

void setup() {
  app.put("/user", &updateUser);
}
```

Respond to a DELETE request to the `/user` route:

```arduino
void removeUser(Request &req, Response &res) {
  res.print("Got a DELETE request at /user");
}

void setup() {
  app.del("/user", &removeUser);
}
```

For more details about routing, see the [routing guide](/{{ page.lang }}/guide/routing.html).
