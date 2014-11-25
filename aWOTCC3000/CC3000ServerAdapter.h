#ifndef CC3000SERVERADAPTER_H_
#define CC3000SERVERADAPTER_H_

#include <Arduino.h>
#include <ServerInterface.h>
#include <CC3000/Adafruit_CC3000.h>
#include <CC3000/Adafruit_CC3000_Server.h>
#include <CC3000ClientAdapter.h>

class CC3000ServerAdapter: public ServerInterface {
public:
	CC3000ServerAdapter(Adafruit_CC3000_Server * server);
	ClientInterface *available();

private:
	Adafruit_CC3000_Server * server_obj;
	virtual inline Adafruit_CC3000_Server *server() {
		return server_obj;
	}
	CC3000ClientAdapter * clientAdapter;
};

#endif /* CC3000SERVERADAPTER_H_ */