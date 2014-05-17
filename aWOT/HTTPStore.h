#ifndef HTTPStore_H_
#define HTTPStore_H_

#include <aJSON.h>
#include <aJsonStore.h>
#include <ClientInterface.h>

// declare a static string
#ifdef __AVR__
#define P(name)   static const unsigned char name[] PROGMEM
#else
#define P(name)   static const unsigned char name[]
#endif

#ifndef HTTP_STORE_METHOD_POST
#define HTTP_STORE_METHOD_POST "POST /"
#endif

#ifndef HTTP_STORE_METHOD_GET
#define HTTP_STORE_METHOD_GET "GET /"
#endif


#ifndef HTTP_STORE_METHOD_PUT
#define HTTP_STORE_METHOD_PUT "PUT /"
#endif


#ifndef HTTP_STORE_METHOD_DELETE
#define HTTP_STORE_METHOD_DELETE "DELETE /"
#endif

#ifndef HTTP_STORE_HTTP_VERSION
#define HTTP_STORE_HTTP_VERSION " HTTP/1.0"
#endif

#ifndef HTTP_STORE_CONTENT_LENGTH
#define HTTP_STORE_CONTENT_LENGTH "Content-Length: "
#endif

#ifndef HTTP_STORE_CONTENT_TYPE_JSON
#define HTTP_STORE_CONTENT_TYPE_JSON "Content-Type: application/json"
#endif

#ifndef HTTP_STORE_CRLF
#define HTTP_STORE_CRLF "\r\n"
#endif

P(httpMethodGet) = "GET /";
P(httpMethodPost) = "POST /";
P(httpMethodPut) = "PUT /";
P(httpMethodDelete) = "DELETE /";
P(httpVersion) = " HTTP/1.0";
P(httpContentLenght) = "Content-Length: ";
P(httpContentTypeJSON) = "Content-Type: application/json";


class HTTPStore: private Stream, public aJsonStore {

public:
	HTTPStore(ClientInterface *clientInterface);



	void saveModel(char * name, aJsonObject* model);
	aJsonObject* findModel(char * name);
	void removeModel(char * name);
	void streamModel(char * name, Stream * stream);
	void saveModelStream(char * name, Stream * stream, int contentLenght);

	void createCollection(char * name);
	aJsonObject* findCollection(char * name);
	void removeCollection(char * name);
	void streamCollection(char * name, Stream * stream);

	void saveModelToCollection(char * name, aJsonObject* model);
	aJsonObject* findModelFromCollection(char * name, int id);
	void removeModelFromCollection(char * name, int id);
	void streamModelFromCollection(char * name, int id, Stream * stream);
	void saveModelStreamToCollection(char * name, Stream * stream, int contentLenght);

	void setServer(IPAddress ip, uint16_t port);




private:

	bool connect();

size_t write(uint8_t);
 int available();
 int read();
 int peek();
 void flush();

	int parseStatus();
	aJsonObject * parseBody();
	void printP(const prog_uchar *str);
	void printCRLF();
	ClientInterface * clientInterface;
	IPAddress ip;
	uint16_t port;
	boolean lastConnected;
	virtual inline ClientInterface *client() { return clientInterface; }




};

#endif
