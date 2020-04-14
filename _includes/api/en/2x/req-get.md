<h3 id='req-get'>Request::get</h3>
<h4 class='variant'>char * get(const char *name);</h4>

Returns a header value by name. Only header values defined for reading with Application::header() will be available. The matching is case insensitive. If the the same header name is defined multiple times in the request the values will be concatenated together using commas.

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
