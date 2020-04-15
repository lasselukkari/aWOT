<h3 id='req-parseInt'>Request::parseInt</h3>
<h4 class='variant'>long parseInt();</h4>
<h4 class='variant'>long parseInt(lookahead LookaheadMode);</h4>
<h4 class='variant'>long parseInt(lookahead LookaheadMode, ignore char);</h4>

Returns the first valid (long) integer number from the current position.

In particular:
* Parsing stops when no characters have been read for a configurable time-out value, a non-digit is read, or it reaches the end of the request.
* If no valid digits were read when the time-out occurs, 0 is returned.

The lookahead defines the mode used to look ahead in the stream for an integer.

Possilbe lookahead mode values are:

* SKIP_ALL: all characters other than digits or a minus sign are ignored when scanning the stream for an integer. This is the default mode.
* SKIP_NONE: Nothing is skipped, and the stream is not touched unless the first waiting character is valid.
* SKIP_WHITESPACE: Only tabs, spaces, line feeds, and carriage returns are skipped.

The ignore char is used to skip the indicated char in the search. Used for example to skip thousands divider.
