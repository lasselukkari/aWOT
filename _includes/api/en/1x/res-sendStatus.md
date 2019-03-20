<h3 id='res.sendStatus'>void sendStatus(int code);</h3>

Send a HTTP full http response matching to the code.

##### Example
```arduino
res.sendStatus(404);
 // is equivalent to 
res.status(404);
res.print(“Not Found”);
```
