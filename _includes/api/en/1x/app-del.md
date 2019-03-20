<h3 id='app.del'>void del(const char* path, Router::Middleware* middleware);</h3>

Routes HTTP DELETE requests to the specified path with the specified Middleware function.

For more information, see the [routing guide](/guide/routing.html).

##### Example
```arduino
void deleteResource(Request &req, Response &res) {
  res.sendStatus(204);
}

app.del("/delete", &deleteResource);
```
