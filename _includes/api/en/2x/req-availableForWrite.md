<h3 id='req-availableForWrite'>Request::availableForWrite</h3>
<h4 class='variant'>int availableForWrite();</h4>

Returns the number of bytes (characters) available for writing in the response buffer without blocking the write operation. 

This function has been exposed from the response class trough the request so that the request can be used as a [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/).
