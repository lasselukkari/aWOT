<h3 id='req.timeout'>bool timeout();</h3>

Returns true if the previous call to read() was timed out. The time to wait for incoming data is defined in `SERVER_READ_TIMEOUT_MS` value.

##### Example
```arduino
while(req.left() && !req.timeout()){
  res.write(req.read());
}
```
