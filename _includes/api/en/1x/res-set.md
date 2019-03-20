<h3 id='res.set'>void set(const char* name, const char* value);</h3>

Sets the response’s HTTP header field to the value. Existing value will be overridden by the new value if the name matches. The match is case-insensitive. The max number of headers is defined in SERVER_MAX_HEADERS.

##### Example
```arduino
res.set("Content-Type", "text/plain");
```
