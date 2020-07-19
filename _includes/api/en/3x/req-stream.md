<h3 id='req-stream'>Request::stream</h3>
<h4 class='variant'>Stream* req.stream();</h4>

Returns a pointer to the raw client stream. You should not normally need to use this. Reading from the returned stream will not add to the the result of bytesRead() and you need to take care of request timeouts and watch out for not reading over the request end.
