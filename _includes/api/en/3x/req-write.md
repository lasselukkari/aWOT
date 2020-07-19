<h3 id='req-write'>Request::write</h3>
<h4 class='variant'>size_t write(const char *str)</h4>
<h4 class='variant'>size_t write(String str)</h4>
<h4 class='variant'>size_t write(const uint8_t *buffer, size_t lenght)</h4>

Writes data to the response. To send the characters representing the digits of a number use the print() function instead.

This function has been exposed from the response class trough the request so that the request can be used as a [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/).