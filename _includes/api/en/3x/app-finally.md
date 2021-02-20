<h3 id='app-finally'>Application::finally</h3>
<h4 class='variant'>void finally(Router::Middleware* middleware);</h4>

This method is like any other app.METHOD(), except it executed always as the last middleware. The middleware is called even if the request does not match any routes or the request has been ended. There can be only one final middleware. If the method is called more than once the previous calls are ineffective.