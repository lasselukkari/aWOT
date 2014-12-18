
#ifndef ESP8266ServerAdapter_H_
#define ESP8266ServerAdapter_H_
#include <Arduino.h>
#include <ServerInterface.h>
#include <ESP8266.h>
#include <ESP8266ClientAdapter.h>

class ESP8266ServerAdapter: public ServerInterface {
public:
	ESP8266ServerAdapter(ESP8266 * esp);
	ClientInterface *available();

private:
	ESP8266 *esp;
	ESP8266ClientAdapter * clientAdapter;
};

#endif /* ESP8266ServerAdapter_H_ */
