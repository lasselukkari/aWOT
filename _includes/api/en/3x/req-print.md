<h3 id='req-print'>Request::print</h3>
<h4 class='variant'>size_t print(const __FlashStringHelper *);</h4>
<h4 class='variant'>size_t print(const String &);</h4>
<h4 class='variant'>size_t print(const char[]);</h4>
<h4 class='variant'>size_t print(char);</h4>
<h4 class='variant'>size_t print(unsigned char, int base);</h4>
<h4 class='variant'>size_t print(int, int base);</h4>
<h4 class='variant'>size_t print(unsigned int, int base);</h4>
<h4 class='variant'>size_t print(long, int base);</h4>
<h4 class='variant'>size_t print(unsigned long, int base);</h4>
<h4 class='variant'>size_t print(double, int = 2);</h4>
<h4 class='variant'>size_t print(const Printable&);</h4>

Prints data as human-readable ASCII text. This command can take many forms. Numbers are printed using an ASCII character for each digit. Floats are similarly printed as ASCII digits, defaulting to two decimal places. Bytes are sent as a single character. Characters and strings are sent as is.

An optional second parameter specifies the base (format) to use; permitted values are BIN(binary, or base 2), OCT(octal, or base 8), DEC(decimal, or base 10), HEX(hexadecimal, or base 16). For floating point numbers, this parameter specifies the number of decimal places to use.


This function has been exposed from the response class trough the request so that the request can be used as a [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/).
