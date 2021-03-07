<h3 id='app-head'>Application::head</h3>
<h4 class='variant'>void head(const char* path, Router::Middleware* middleware);</h4>
<h4 class='variant'>void head(Router::Middleware* middleware);</h4>

Routes HTTP HEAD requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example
```arduino
void index(Request &req, Response &res) {
  res.set("etag", "5ebd65ed-2b957");
  res.status(200);
}

app.head("/index.html", &index);
```
