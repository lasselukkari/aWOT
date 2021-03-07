<h3 id='router-head'>Router::head</h3>
<h4 class='variant'>void head(const char* urlPattern, Middleware* middleware);</h4>
<h4 class='variant'>void head(Middleware* middleware);</h4>

Routes a HEAD request to a Middleware when the request url matches the url pattern. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).
