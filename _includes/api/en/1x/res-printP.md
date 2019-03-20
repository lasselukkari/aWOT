<h3 id='res.printP'>
void printP(const char *string);
void printP(const unsigned char *string);
</h3>

Output a string stored in the program memory, usually defined with the P macro. Using this will save you from wasting your precious RAM for static strings.

Ther overloaded version is for handling signed char strings
