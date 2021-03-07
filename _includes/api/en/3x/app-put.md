<h3 id='app-put'>Application::put</h3>
<h4 class='variant'>void put(const char* path, Router::Middleware* middleware);</h4>
<h4 class='variant'>void put(Router::Middleware* middleware);</h4>

Routes HTTP PUT requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example

```arduino
void putSomething(Request &req, Response &res) {
  char body[100];
  req.readBytes(type, 100);
}

app.put("/example", &putSomething);
```
