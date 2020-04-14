<h3 id='res-status'>Response::status</h3>
<h4 class='variant'>void status(int code);</h4>

Sends the initial status line of the HTTP the response. After calling the function new headers can be still set to the response before first write method is called.

##### Example

```arduino
res.status(404);
res.print("Pagina no encontrada");
```
