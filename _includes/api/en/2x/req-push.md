<h3 id='req-push'>Request::push</h3>
<h4 class='variant'>void push(uint8_t ch);</h4>

Puts a byte back to the read buffer. Undo of read(). If the internal buffer gets full the last byte is replaced over and over again. The internal buffer size is defined in the SERVER_PUSHBACK_BUFFER_SIZE value.
