<h3 id='req.get'>char * get(const char *name);</h3>

Returns a header value by name. Only header values marked for reading with Application::header() will be available. The matching is case insensitive.

##### Example
```arduino
char contentType[100];
void headers(Request &req, Response &res) {
  char * type = req.get("Content-Type");
  // => "text/plain"
}

app.header("Content-Type", contentType, 100);
app.use(&headers);
```
