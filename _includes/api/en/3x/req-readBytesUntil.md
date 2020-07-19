<h3 id='req-readBytesUntil'>Request::readBytesUntil</h3>
<h4 class='variant'>int readBytesUntil(char terminator, char *buffer, size_t length)</h4>

Reads characters from a stream into a buffer. The function terminates if the terminator character is detected, the determined length has been read, it times out, or it reaches the end of the request. The function returns the characters up to the last character before the supplied terminator. The terminator itself is not returned in the buffer.

The function returns the number of bytes placed in the buffer. A 0 means no valid data was found.
