<h3 id='res.get'>const char * get(const char *name);</h3>

Returns the HTTP response header specified by name if it has been previously set. The match is case-insensitive.

##### Example
```arduino
res.get('Content-Type');
// => "text/plain"
```
