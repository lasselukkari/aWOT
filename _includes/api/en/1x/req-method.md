<h3 id='req.method'>MethodType method();</h3>

Returns the MethodType type of the incoming http request.

##### Example
```arduino
void noDelete(Request &req, Response &res) {
  if (req.method() == Request::DELETE) {
    res.sendStatus(405);
  }
}

app.all("/nodelete", &noDelete);
```