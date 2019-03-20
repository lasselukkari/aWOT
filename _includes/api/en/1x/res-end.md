<h3 id='res.end'>void end();</h3>

Ends the response process. All further write operations will be discarded. This will also prevent any pending Middleware functions in the chain from being executed.

##### Example
```arduino
void auth(Request &req, Response &res) {
  char * authHeader = req.get("Authorization");

  if (strcmp(authHeader, "Basic c3VwZXI6YWRtaW4=") != 0) { // super:admin in base64
    res.set("WWW-Authenticate", "Basic realm=\"Secret Area\"");
    res.sendStatus(401);
    res.end();
  }
}
```
