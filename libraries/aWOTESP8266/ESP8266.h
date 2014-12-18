
#ifndef ESP8266_H_
#define ESP8266_H_
#include <Arduino.h>

#ifndef ESP8266_RESET_DELAY
#define ESP8266_RESET_DELAY 3000
#endif

#ifndef ESP8266_BEGIN_DELAY
#define ESP8266_BEGIN_DELAY 10000
#endif

#ifndef ESP8266_CONNECT_DELAY
#define ESP8266_CONNECT_DELAY 60000
#endif


#ifndef ESP8266_TCP_OPEN_DELAY
#define ESP8266_TCP_OPEN_DELAY 5000
#endif

#ifndef ESP8266_NEXT_CHAR_DELAY
#define ESP8266_NEXT_CHAR_DELAY 5
#endif

#ifndef ESP8266_OPEN_DELAY
#define ESP8266_OPEN_DELAY 5
#endif


#ifndef ESP8266_WRITE_DELAY
#define ESP8266_WRITE_DELAY 50
#endif

#ifndef ESP8266_COMMAND_DELAY
#define ESP8266_COMMAND_DELAY 100
#endif



class ESP8266 {

public:
	ESP8266(Stream *serial, char * buffer);
	bool begin(byte mode);
	bool connect(char * ssid, char * pwd);
	bool serve(int port);
	void reset();
	bool sendReponse();
	char * IP();
	char * apIP();

	size_t write(uint8_t b);
	size_t write(const char *str);
	size_t write(const uint8_t *buf, size_t size);
	bool connected();
	int read();
	void flush();
	void stop();
	int available();
	int print(const char* str);
	int println(const char* str);
	int println();
	int print(uint16_t b);
	int println(const int aHeaderValue);
	int read(uint8_t *buf, size_t size);
	int peek();

private:
	enum State {BEGUN, ECHODISABLED, MODESET, MUXSET, READY, READING};
	State state;
	Stream *serial;

	bool ack;
	int contentLeft;
	unsigned long gate;
	char * buffer;
	int contentLength;
	char ipBuffer[16];
	int socket;
	char * response;
	char * ssid;
	char * pwd;
	byte mode;
	int port;

};

#endif /* ESP8266ServerAdapter_H_ */
