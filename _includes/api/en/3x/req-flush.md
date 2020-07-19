<h3 id='req-flush'>Request::flush</h3>
<h4 class='variant'></h4>

Flushes outgoing reposen buffer and waits until all outgoing data in buffer have been sent. This is called internally before the client is disconnected at the end of the request.

This function has been exposed from the response class trough the request so that the request can be used as a [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/).
