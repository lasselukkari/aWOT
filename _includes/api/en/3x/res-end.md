<h3 id='res-end'>Response::end</h3>
<h4 class='variant'>void end();</h4>

After calling end no further middleware functions will be executed.

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
