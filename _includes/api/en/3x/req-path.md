<h3 id='req-path'>Request::path</h3>
<h4 class='variant'>char * path();</h4>

Returns a null terminated string containing the path part of the request URL.

##### Example
```arduino
// example.com/users?sort=desc
req.path()
// => "/users"
```
