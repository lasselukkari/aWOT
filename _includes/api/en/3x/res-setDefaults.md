<h3 id='res-setDefaults'>Response::setDefaults</h3>
<h4 class='variant'>void setDefaults();</h4>

The function is normally called internally when the first call to writing the body is happens. If `Content-Type` header has not been set it will be set to `text/plain`. If `Connnection` header has been set to `keep-alive` and `Content-Length` has not been set the `Transfer-Encoding` will set to `chunked`. If `Connnection` header has been not set to `keep-alive` it will set it to `close`.
