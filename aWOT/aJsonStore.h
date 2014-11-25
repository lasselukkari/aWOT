#ifndef AJSONSTORE_H_
#define AJSONSTORE_H_

#include <aJSON.h>
#include <Stream.h>

class aJsonStore {

public:

	virtual void saveModel(char * name, aJsonObject* model);
	virtual aJsonObject* findModel(char * name);
	virtual void removeModel(char * name);
	virtual void findModel(char * name, Stream * stream);
	virtual void saveModel(char * name, Stream * stream, int contentLenght);

	virtual void createCollection(char * name);
	virtual aJsonObject* findCollection(char * name);
	virtual void removeCollection(char * name);
	virtual void findCollection(char * name, Stream * stream);

	virtual void saveToCollection(char * name, aJsonObject* model);
	virtual aJsonObject* findFromCollection(char * name, int id);
	virtual void removeFromCollection(char * name, int id);
	virtual void findFromCollection(char * name, int id, Stream * stream);
	virtual void saveToCollection(char * name, Stream * stream, int contentLenght);

};

#endif /* AJSONSTORE_H_ */


