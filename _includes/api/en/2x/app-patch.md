<h3 id='app-patch'>Application::patch</h3>
<h4 class='variant'>void patch(const char* path, Router::Middleware* middleware);</h4>

Routes HTTP PATCH requests to the specified path with the specified Middleware function.

For more information, see the [routing guide](/guide/routing.html).

##### Example

```arduino
void patchSomething(Request &req, Response &res) {
  char body[100];
  req.readBytes(type, 100);
}

app.patch("/example", &patchSomething);
```
