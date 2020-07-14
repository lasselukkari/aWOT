<h3 id='res-headersSent'>Response::headersSent</h3>
<h4 class='variant'>bool headersSent();</h4>

Returns true if the response headers have been already sent. After headers have been sent all writes will go to the request body. Calling any write method will cause headers to be sent automatically.
