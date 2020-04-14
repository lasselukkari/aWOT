<h3 id='req-findUntil'>Request::findUntil</h3>
<h4 class='variant'>req.findUntil(char target, char terminal);</h4>

Reads data from the stream until the target string of given length or terminator string is found, it times out, or it reaches the end of the request.

The function returns true if target string is found, false if timed out.
