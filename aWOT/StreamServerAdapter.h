
#ifndef StreamServerAdapter_H_
#define StreamServerAdapter_H_
#include <Arduino.h>
#include <ServerInterface.h>
#include <StreamClientAdapter.h>

class StreamServerAdapter: public ServerInterface {
public:
	StreamServerAdapter(Stream *serial);
	ClientInterface *available();

private:
	Stream *streamPointer;
	StreamClientAdapter * clientAdapter;
	virtual inline Stream *stream() {
		return streamPointer;
	}
};

#endif /* StreamServerAdapter_H_ */
