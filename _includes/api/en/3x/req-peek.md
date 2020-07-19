<h3 id='req-peek'>Request::peek</h3>
<h4 class='variant'>int peek();</h4>

Returns the next byte of incoming data without removing it from the internal buffer. Successive calls to peek() will return the same character, as will the next call to read().
