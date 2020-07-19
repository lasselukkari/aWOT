<h3 id='req-timedout'>Request::timedout</h3>
<h4 class='variant'>bool timedout();</h4>

Returns true if the previous call to read() was timed out.

##### Example
```arduino
while(req.left() && !req.timedout()){
  res.write(req.read());
}
```
