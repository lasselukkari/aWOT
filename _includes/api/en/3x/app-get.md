<h3 id='app-get'>Application::get</h3>
<h4 class='variant'>void get(const char* path, Router::Middleware* middleware);</h4>
<h4 class='variant'>void get(Router::Middleware* middleware);</h4>

Routes HTTP GET requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example
```arduino
void index(Request &req, Response &res) {
  res.print("Hello World!");
}

app.get("/index.html", &index);
```
