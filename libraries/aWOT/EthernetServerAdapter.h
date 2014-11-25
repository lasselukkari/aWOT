#ifndef ETHERNETSERVERADAPTER_H_
#define ETHERNETSERVERADAPTER_H_

#include <Arduino.h>
#include <ServerInterface.h>
#include <EthernetServer.h>
#include <EthernetClientAdapter.h>

class EthernetServerAdapter: public ServerInterface {
public:
	EthernetServerAdapter(EthernetServer * server);
	ClientInterface *available();

private:
	EthernetServer * server_obj;
	virtual inline EthernetServer *server() {
		return server_obj;
	}
	EthernetClientAdapter * clientAdapter;
};

#endif /* ETHERNETSERVERADAPTER_H_ */
