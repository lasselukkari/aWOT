#ifndef AJSONSTORE_H_
#define AJSONSTORE_H_

#include <aJSON.h>
#include <Stream.h>

class aJsonStore {

public:




	virtual void saveModel(char * name, aJsonObject* model);
	virtual aJsonObject* findModel(char * name);
	virtual void removeModel(char * name);
	virtual void streamModel(char * name, Stream * stream);
	virtual void saveModelStream(char * name, Stream * stream, int contentLenght);

	virtual void createCollection(char * name);
	virtual aJsonObject* findCollection(char * name);
	virtual void removeCollection(char * name);
	virtual void streamCollection(char * name, Stream * stream);

	virtual void saveModelToCollection(char * name, aJsonObject* model);
	virtual aJsonObject* findModelFromCollection(char * name, int id);
	virtual void removeModelFromCollection(char * name, int id);
	virtual void streamModelFromCollection(char * name, int id, Stream * stream);
	virtual void saveModelStreamToCollection(char * name, Stream * stream, int contentLenght);

};

#endif /* AJSONSTORE_H_ */


