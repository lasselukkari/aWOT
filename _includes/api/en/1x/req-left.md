<h3 id='req.left'>int left();</h3>

Returns the number bytes left to read in the request body. The initial value is taken from the Content-Length header.

##### Example
```arduino
while(req.left() && !req.timeout()){
  res.write(req.read());
}
```

