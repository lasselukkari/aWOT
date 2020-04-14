<h3 id='app-route'>Application::route</h3>
<h4 class='variant'>void route(Router * router);</h4>

Mounts a [Router](#router) instance to the application.

##### Example

```arduino
Application app;
Router cats("/cats");

void looooong(Request &req, Response &res) {
  res.print("looooong cat is long!");
}

void ceiling(Request &req, Response &res) {
  res.print("ceiling cat is watching you debug!");
}

void nyannyan(Request &req, Response &res) {
  for (int i = 0; i < 100; i++) {
      res.print("nyan ");
  }
}

cats.get("/long", &looooong);
cats.get("/ceiling", &ceiling);
cats.get("/nyan", &nyannyan);

app.route(&cats);
```
