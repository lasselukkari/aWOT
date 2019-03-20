<h3 id='req.path'>char * path();</h3>

Returns a null terminated string contains the path part of the request URL.

##### Example
```arduino
// example.com/users?sort=desc
req.path()
// => "/users"
```

<div class="doc-box doc-info" markdown="1">
When called from a middleware, the mount point is included in `req.path`.
</div>
