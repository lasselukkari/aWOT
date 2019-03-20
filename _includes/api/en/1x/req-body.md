<h3 id='req.body'>bool body(uint8_t *buffer, int bufferLength);</h3>

Reads the request body to a buffer. The buffer must have space for null termination. The function returns true if was able to read the the amount of data specified in the Content-Length headers. If the client timed out or the body did not fit the buffer completely it returns false.

##### Example
```arduino
byte buffer[100];

if(!req.body(buffer, 100)){
  return res.sendStatus(400);
}
```
