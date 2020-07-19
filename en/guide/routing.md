---
layout: page
title: aWOT routing
menu: guide
lang: en
redirect_from: "/guide/routing.html"
---

# Routing

_Routing_ refers to how an application's endpoints (URIs) respond to client requests.
For an introduction to routing, see [Basic routing](/{{ page.lang }}/starter/basic-routing.html).

You define routing using methods of the aWOT `Application` instance object that correspond to HTTP methods;
for example, `app.get()` to handle GET requests and `app.post()` to handle POST requests.  For a full list,
see [req.methodType](/{{ page.lang }}/3x/api.html#req-methodType).  You can also use [app.all()](/{{ page.lang }}/3x/api.html#app-all) to handle all HTTP methods and [app.use()](/{{ page.lang }}/3x/api.html#app-use) to
specify middleware as the callback function (See [Using middleware](/{{ page.lang }}/guide/using-middleware.html) for details).

These routing methods specify a callback function (sometimes called "handler functions") called when the application receives a request to the specified route (endpoint) and HTTP method.  In other words, the application "listens" for requests that match the specified route(s) and method(s), and when it detects a match, it calls the specified callback function.


The following code is an example of a very basic route.

```arduino
void index(Request &req, Response &res) {
  res.print("Hello World!");
}

void setup() {
  app.get("/", &index);
}
```

<h2 id="route-methods">Route methods</h2>

A route method is derived from one of the HTTP methods, and is attached to an instance of the `aWOT` class.

The following code is an example of routes that are defined for the GET and the POST methods to the root of the app.

```arduino
void getIndex(Request &req, Response &res) {
  res.print("GET request to the homepage");
}

void postIndex(Request &req, Response &res) {
  res.print("POST request to the homepage");
}

void setup() {
  // GET method route
  app.get("/", &getIndex);
  // POST method route
  app.post("/", &postIndex);
}
```

aWOT supports methods that correspond to all HTTP request methods: `get`, `post`, and so on.
For a full list, see [req.MethodType](/{{ page.lang }}/3x/api.html#req-methodType).

There is a special routing method, `app.use()`, used to load middleware functions at a path for all HTTP request methods.  For example, the following handler is executed for requests to the route "/secret" whether using GET, POST, PUT, DELETE, or any other HTTP request method supported.

```arduino
void secret(Request &req, Response &res) {
  Serial.println("Accessing the secret section ...");
}

void setup() {
  app.use("/secret", &secret);
}
```

<h2 id="route-paths">Route paths</h2>

Route paths, in combination with a request method, define the endpoints at which requests can be made. Route paths can be strings or string patterns.

<div class="doc-box doc-warn" markdown="1">
Query strings are not part of the route path.
</div>

Here are some examples of route paths based on strings.

This route path will match requests to the root route, `/`.

```arduino
void index(Request &req, Response &res) {
  res.print("root");
}

void setup() {
  app.get("/", &index);
}
```

This route path will match requests to `/about`.

```arduino
void about(Request &req, Response &res) {
  res.print("about");
}

void setup() {
  app.get("/about", &about);
}
```

This route path will match requests to `/random.text`.

```arduino
void random(Request &req, Response &res) {
  res.print("random.text");
}

void setup() {
  app.get("/random.text", &random);
}
```

<h3 id="route-parameters">Route parameters</h3>

Route parameters are named URL segments that are used to capture the values specified at their position in the URL. The captured values can be accessed by req.route() method, with the name of the route parameter specified in the path as their respective keys.

```
Route path: /users/:userId/books/:bookId
Request URL: http://arduino.ip/users/34/books/8989
char user[10];
req.route("userId", user, 10);
// => "34"
char book[10];
req.route("bookId", book, 10);
// => "8989"
```

To define routes with route parameters, simply specify the route parameters in the path of the route as shown below.

```arduino
void usersBook(Request &req, Response &res) {
  char user[10];
  char book[10];
  
  req.route("userId", user, 10);
  req.route("bookId", book, 10);

  res.print("User: ");
  res.println(user);

  res.print("Book: ");
  res.println(book);
}

void setup() {
  router.get("/users/:userId/books/:bookId", &usersBook);;
} 
```

<h2 id="aWOT-router">Router</h2>

Use the `Router` class to create modular, mountable route handlers. A `Router` instance is a complete middleware and routing system; for this reason, it is often referred to as a "mini-app".

The following example creates a router, loads a middleware functions in it, defines some routes, and mounts the router instance on a path in the main app.

```arduino
Application app;
Router cats;

void catIndex(Request &req, Response &res) {
  res.print("There are 3 cats: /long, /ceiling and /nyan");
}

void looooong(Request &req, Response &res) {
  res.success("text/html");
  res.print("looooong cat is long!");
}

void ceiling(Request &req, Response &res) {
  res.success("text/html");
  res.print("ceiling cat is watching you debug!");
}

void nyannyan(Request &req, Response &res) {
  res.success("text/html");

  for (int i = 0; i < 100; i++) {
      res.print("nyan ");
  }
}

void setup() {
  cats.get("/", &catIndex);
  cats.get("/long", &looooong);
  cats.get("/ceiling", &ceiling);
  cats.get("/nyan", &nyannyan);

  app.use("/cats", &cats);
}
```

The app will now be able to handle requests to `/cats/` and `/cats/long` and so on.
