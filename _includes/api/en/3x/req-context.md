<h3 id='req-context'>Request::context</h3>
<h4 class='variant'>void* context;</h4>

The request context pointer can be used for providing arbitrary data to the middleware chain. The pointer can be set or modified by any middlwares and the initial value can be provided as parameters to the [Application:process](/en/3x/api.html#app-process) function.

##### Example
```arduino
struct context {
  IPAddress ip;
};

void index(Request &req, Response &res) {
  context* ctx = (context*)req.context;
  res.print("Your IP is: ");
  res.println(ctx->ip);
}

context ctx = { ip: client.remoteIP(), start: millis() };

app.process(&client, &ctx);
```
