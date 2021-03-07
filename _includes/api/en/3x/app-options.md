<h3 id='app-options'>Application::options</h3>
<h4 class='variant'>void options(const char* path, Router::Middleware* middleware);</h4>
<h4 class='variant'>void options(Router::Middleware* middleware);</h4>

Routes HTTP OPTIONS requests to the specified path with the specified Middleware function. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).

##### Example

```arduino
void cors(Request &req, Response &res) {
  res.set("Access-Control-Allow-Origin", "*");
  res.set("Access-Control-Allow-Methods", "GET, HEAD");
  res.sendStatus(204);
}

app.options("/js/app.js", &cors);
```
