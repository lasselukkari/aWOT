<h3 id='req.peek'>int peek();</h3>

Returns the next byte of incoming data without removing it from the internal buffer. That is, successive calls to peek() will return the same character, as will the next call to read(). peek() inherits from the Stream class.