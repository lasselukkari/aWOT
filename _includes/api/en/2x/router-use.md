<h3 id='router-use'>Router::use</h3>
<h4 class='variant'>void use(Middleware* middleware);</h4>
<h4 class='variant'>void use(Router * router);</h4>

Makes the Middleware executed when the request URL beginning matches the prefix of the router. The overloaded version is used for nesting routers.

For more information, see the [routing guide](/guide/routing.html).

