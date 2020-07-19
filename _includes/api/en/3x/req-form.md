<h3 id='req-form'>Request::form</h3>
<h4 class='variant'>bool form(char *name, int nameLength, char *value, int valueLength);</h4>

Reads the next key value pair from the request body. The function expects the request content type to be as application/x-www-form-urlencoded.

Returns true if the there was a value in the body. Returns false if there is no name value pair to read or the content were too long for the buffers.

##### Example
```arduino
char name[10];
char value[100];

if(!req.form(name, 10, value, 100)){
  return res.sendStatus(400);
}
```
