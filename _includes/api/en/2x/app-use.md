<h3 id='app-use'>Application::use</h3>
<h4 class='variant'>void use(Router::Middleware* middleware);</h4>
<h4 class='variant'>void use(Router * router);</h4>

This method is like any standard app.METHOD() methods, except it matches all HTTP verbs and routes. The overloaded version mounts a Router instance to the application.

##### Example Middleware

```arduino
void logRequest(Request &req, Response &res) {
  Serial.print(millis());
  Serial.print(": ");

  switch (req.method()) {
    case  Request::GET: {
        Serial.print("GET ");
        break;
      }
    case  Request::POST: {
        Serial.print("POST ");
        break;
      }
    case  Request::PUT: {
        Serial.print("PUT ");
        break;
      }
    case  Request::PATCH: {
        Serial.print("GET ");
        break;
      }
    case  Request::DELETE: {
        Serial.print("DELETE ");
        break;
      }
    default: {}
  }

  Serial.println(req.path());
}

app.use(&logRequest);
```

##### Example Router

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

app.use(&cats);
```