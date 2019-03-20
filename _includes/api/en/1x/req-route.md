<h3 id='req.route'>
    bool route(const char *name, char *buffer, int bufferLength);<br/>
    bool route(int number, char *buffer, int bufferLength);
</h3>

The firt version of the method reads the route parameter defined by name to the buffer. The buffer must have space for null termination. Returns true if the parameter was found and the buffer had enough space for the value. Returns false if the parameter is not found or the value was too long for the buffer.

The second overload works exaclty the same way but it uses an index number for the look up.

##### Example
```arduino
// HTTP GET /cats/lolcat
void routeParams(Request &req, Response &res) {
  char catId[64];
  req.route("catId", catId, 64);
  // => "lolcat"
}

void setup() {
  app.get("/cats/:catId", &routeParams);
}
```