<h3 id='req-readBytes'>Request::readBytes</h3>
<h4 class='variant'>int readBytes(uint8_t *buffer, int bufferLength);</h4>

Read characters from the request into a buffer. The function terminates if the determined length has been read, it times out, or it reaches the end of the request.

The functionn returns the number of bytes placed in the buffer. A 0 means no valid data was found.

##### Example
```arduino
byte buffer[100];

if(!req.readBytes(buffer, 100)){
  return res.sendStatus(400);
}
```
