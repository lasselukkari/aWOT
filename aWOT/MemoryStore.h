#ifndef MEMORYSTORE_H_
#define MEMORYSTORE_H_

#include <aJSON.h>
#include <aJsonStore.h>

class MemoryStore: public aJsonStore {

public:
	MemoryStore();



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



private:
	aJsonObject* root;
	int nextId;

};

#endif
