<h3 id='req.read'>int read();</h3>
Reads a byte from the incoming stream. Returns -1 if the there is no data available during the the time specified in the `SERVER_READ_TIMEOUT_MS` setting.


##### Example
```arduino
// HTTP GET /cats/lolcat
void routeParams(Request &req, Response &res) {
  char catId[64];
  req.route("catId", catId, 64);

  res.success("text/plain");
  res.print(catId);
}

void setup() {
  // other setup

  app.get("/cats/:catId", &routeParams);
}
```