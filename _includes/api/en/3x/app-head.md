<h3 id='app-head'>Application::head</h3>
<h4 class='variant'>void head(const char* path, Router::Middleware* middleware);</h4>

Routes HTTP HEAD requests to the specified path with the specified Middleware function.

For more information, see the [routing guide](/guide/routing.html).

##### Example
```arduino
void index(Request &req, Response &res) {
  res.set("etag", "5ebd65ed-2b957");
  res.status(200);
}

app.head("/index.html", &index);
```
