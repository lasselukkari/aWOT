<h3 id='res-sendStatus'>Response::sendStatus</h3>
<h4 class='variant'>void sendStatus(int code);</h4>

Send a full HTTP response matching to the status code.

##### Example

```arduino
res.sendStatus(404);
 // is equivalent to 
res.status(404);
res.print(“Not Found”);
```
