<h3 id='res.status'>void status(int code);</h3>

Sends the initial status line of the HTTP the response. After calling the function new headers can be still set to the response before first write method is called.

##### Example
```arduino
res.status(404);
res.print("Pagina no encontrada");
```
