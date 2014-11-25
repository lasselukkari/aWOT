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
	void findModel(char * name, Stream * stream);
	void saveModel(char * name, Stream * stream, int contentLenght);

	void createCollection(char * name);
	aJsonObject* findCollection(char * name);
	void removeCollection(char * name);
	void findCollection(char * name, Stream * stream);

	void saveToCollection(char * name, aJsonObject* model);
	aJsonObject* findFromCollection(char * name, int id);
	void removeFromCollection(char * name, int id);
	void findFromCollection(char * name, int id, Stream * stream);
	void saveToCollection(char * name, Stream * stream, int contentLenght);



private:
	aJsonObject* root;
	int nextId;

};

#endif
