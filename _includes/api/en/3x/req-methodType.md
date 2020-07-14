<h3 id='req-methodType'>Request::MethodType</h3>
<h4 class='variant'>enum MethodType {GET, HEAD, POST, PUT, DELETE, PATCH, OPTIONS, ALL, USE}</h4>

The incoming HTTP request method type. HEAD requests are converted to GET requests but only the headers are sent to the client. The method types ALL and USE are used when defining middleware handlers to a Router.

