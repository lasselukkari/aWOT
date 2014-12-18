Uses a modiefied firmware that buffers the incoming requests to the ESP8622. Buffer size is 1024 chars/request.

Firmware has a new AT-command AT+CIPREAD which makes the ESP to print one buffer at time to the serial port.

Format is the same as with the normal AT firmware.

Burn the firmware with esptool

eg. esptool.py --port /dev/tty.usbmodem14221 write_flash 0x00000 0x00000.bin 0x40000 0x40000.bin

Please note that currently the stability of ESP8266 is really limited. Random resets under heavy loads are common, 
but the library will try to recover by resetting the connection if errors occur.