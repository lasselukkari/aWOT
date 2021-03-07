<h3 id='router-del'>Router::del()</h3>
<h4 class='variant'>void del(const char* urlPattern, Middleware* middleware);</h4>
<h4 class='variant'>void del(Middleware* middleware);</h4>

Routes a DELETE request to a Middleware when the request url matches the url pattern. If no path is specified the matching is done based on the HTTP verb.

For more information, see the [routing guide](/guide/routing.html).
