<h3 id='req-query'>Request::query</h3>
<h4 class='variant'>char * query();</h4>
<h4 class='variant'>bool query(const char *name, char *buffer, int bufferLength);</h4>

Without any parameters the method returns a null terminated string containing the query part of the request URL.

The parametrized version reads the request parameter defined by the name to the buffer. The buffer must have space for null termination. Returns true if the parameter was found and the buffer had enough space for the value. Returns false if the parameter is not found or the value was too long for the buffer.

##### Example
```arduino
/cats?type=lolcat
req.query();
// => "type=lolcat"

char type[64];
req.query("type", type, 64);
// => "lolcat"
```
