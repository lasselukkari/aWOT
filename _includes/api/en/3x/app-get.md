<h3 id='app-get'>Application::get</h3>
<h4 class='variant'>void get(const char* path, Router::Middleware* middleware);</h4>

Routes HTTP GET requests to the specified path with the specified Middleware function.

For more information, see the [routing guide](/guide/routing.html).

##### Example
```arduino
void index(Request &req, Response &res) {
  res.print("Hello World!");
}

app.get("/index.html", &index);
```
