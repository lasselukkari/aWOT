<h3 id='req.available'>int available();</h3>

Returns the number of bytes available for reading. That is, the amount of data that has been written to the client by the server it is connected to. It is not the expected length of the request body defined in the Content-Length header. The function inherits from the Stream utility class.
