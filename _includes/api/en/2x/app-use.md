<h3 id='app-use'>Application::use</h3>
<h4 class='variant'>void use(Router::Middleware* middleware);</h4>

This method is like any standard app.METHOD() methods, except it matches all HTTP verbs and routes.

##### Example

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
