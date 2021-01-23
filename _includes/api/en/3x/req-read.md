<h3 id='req-read'>Request::read</h3>
<h4 class='variant'>int read();</h4>
<h4 class='variant'>int read(uint8_t* buf, size_t size);</h4>

Reads a byte from the incoming stream. Returns -1 if no data is available.

The buffered version returns the number of bytes read and -1 in case of error.
