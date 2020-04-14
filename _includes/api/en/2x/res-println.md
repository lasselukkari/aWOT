<h3 id='res-println'>Response::println</h3>
<h4 class='variant'>size_t println(const __FlashStringHelper *);</h4>
<h4 class='variant'>size_t println(const String &s);</h4>
<h4 class='variant'>size_t println(const char[]);</h4>
<h4 class='variant'>size_t println(char);</h4>
<h4 class='variant'>size_t println(unsigned char, int = DEC);</h4>
<h4 class='variant'>size_t println(int, int = DEC);</h4>
<h4 class='variant'>size_t println(unsigned int, int = DEC);</h4>
<h4 class='variant'>size_t println(long, int = DEC);</h4>
<h4 class='variant'>size_t println(unsigned long, int = DEC);</h4>
<h4 class='variant'>size_t println(double, int = 2);</h4>
<h4 class='variant'>size_t println(const Printable&);</h4>
<h4 class='variant'>size_t println(void);</h4>

Prints data as human-readable ASCII text followed by a carriage return character (ASCII 13, or '\r') and a newline character (ASCII 10, or '\n'). This command takes the same forms as reponse.print().
