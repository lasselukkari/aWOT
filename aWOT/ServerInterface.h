#ifndef SERVERINTERFACE_H_
#define SERVERINTERFACE_H_

class ClientInterface;

class ServerInterface {
public:
	virtual ClientInterface *available();
};

#endif /* SERVERINTERFACE_H_ */
