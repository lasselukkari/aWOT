<h3 id='app-notFound'>Application::notFound</h3>
<h4 class='variant'>void notFound(Router::Middleware* middleware);</h4>

By defualt the server send a generic 404 - Not Found page if none of the routes match to the request. The notFound mehod can be use to set up a custon middleware handler to be executed instead.
