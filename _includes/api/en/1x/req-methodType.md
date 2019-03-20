<h3 id='req.methodType'>enum MethodType {GET, HEAD, POST, PUT, DELETE, PATCH, OPTIONS, ALL, USE}</h3>

The incoming HTTP request method type. HEAD requests are converted to GET requests but only the headers are sent to the client. The method types ALL and USE are used when defining middleware handlers to a Router.
