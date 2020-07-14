<h3 id='app-post'>Application::post</h3>
<h4 class='variant'>void post(const char* path, Router::Middleware* middleware);</h4>

Routes HTTP POST requests to the specified path with the specified Middleware function.

For more information, see the [routing guide](/guide/routing.html).

##### Example

```arduino
void form(Request &req, Response &res) {
  char name[10];
  char value[10];

  while (req.left()) {
    if (!req.form(name, 10, value, 10)) {
      return res.sendStatus(400);
    }

    res.print(name);
    res.print(":");
    res.println(value);
  }
}

app.post("/form", &form);
```
