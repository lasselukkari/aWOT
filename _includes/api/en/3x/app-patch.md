<h3 id='app-patch'>Application::patch</h3>
<h4 class='variant'>void patch(const char* path, Router::Middleware* middleware);</h4>
<h4 class='variant'>void patch(Router::Middleware* middleware);</h4>

Routes HTTP PATCH requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example

```arduino
void patchSomething(Request &req, Response &res) {
  char body[100];
  req.readBytes(type, 100);
}

app.patch("/example", &patchSomething);
```
