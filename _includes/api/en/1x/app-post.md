<h3 id='app.post'>void post(const char* path, Router::Middleware* middleware);</h3>

Routes HTTP POST requests to the specified path with the specified Middleware function.

For more information, see the [routing guide](/guide/routing.html).

##### Example
```arduino
void form(Request &req, Response &res) {
 while (req.contentLeft()) {
    res.print(name);
    res.print(":");
    res.print(value);
    if (req.contentLeft()) {
      res.print(" ");
    }
  }
}

app.post("/form", &form);
```
