<h3 id='app.options'>void options(const char* path, Router::Middleware* middleware);</h3>

Routes HTTP OPTIONS requests to the specified path with the specified Middleware function.

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
