<h3 id='req-left'>Request::left</h3>
<h4 class='variant'>int left();</h4>

Returns the number bytes left to read in the request body. The initial value is taken from the Content-Length header.

##### Example
```arduino
while(req.left() && !req.timedout()){
  res.write(req.read());
}
```
