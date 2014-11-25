Proof of concept level prototype for using HTTP services over serial connections in PhoneCap applications.

Redirects the HTTP requests to phones Bluetooth serial and parses the response.

Transparent when works, often doesn't. 

##Usage

- Import the BT library to your project. https://github.com/don/BluetoothSerial
- Include jQuery
- Include the btajax.js 
- $.ajax.useBluetooth = true;
