<h3 id='req-parseFloat'>Request::parseFloat</h3>
<h4 class='variant'>float parseFloat();</h4>
<h4 class='variant'>float parseFloat(lookahead LookaheadMode);</h4>
<h4 class='variant'>float parseFloat(lookahead LookaheadMode, ignore char);</h4>

Returns the first valid floating point number from the current position. The function is terminated by the first character that is not a floating point number. The function terminates if it times out or it reaches the end of the request.

The lookahead defines the mode used to look ahead in the stream for a floating point number. 

Possilbe lookahead mode values are:

* SKIP_ALL: all characters other than a minus sign, decimal point, or digits are ignored when scanning the stream for a floating point number. This is the default mode.
* SKIP_NONE: Nothing is skipped, and the stream is not touched unless the first waiting character is valid.
* SKIP_WHITESPACE: Only tabs, spaces, line feeds, and carriage returns are skipped.

The ignore char is used to skip the indicated char in the search. Used for example to skip thousands divider.
