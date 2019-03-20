<h3 id='res.write'>
size_t write(uint8_t ch);<br/>
size_t write(uint8_t *buffer, size_t bufferLength);
</h3>

The first version writes a single byte to the response. The function inherits from the Stream utility class. The other version writes a buffer to the response. 

Returns the number of bytes written. The function inherits from the Stream utility class.