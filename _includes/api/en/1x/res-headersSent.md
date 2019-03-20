<h3 id='res.headersSent'>bool headersSent();</h3>

Returns true if the response headers have been already sent. After headers have been sent all writes will go to the request body. Calling any write method will cause headers to be sent automatically. 