<h3 id='router-options'>Router::options</h3>
<h4 class='variant'>void options(const char* urlPattern, Middleware* middleware);</h4>
<h4 class='variant'>void options(Middleware* middleware);</h4>

Routes a OPTIONS request to a Middleware when the request url matches the url pattern. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).
