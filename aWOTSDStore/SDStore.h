#ifndef SDSTORE_H_
#define SDSTORE_H_

#include <arduino.h>
#include <aJSON.h>
#include <SD.h>
#include <aJsonStore.h>

class SDStore: public aJsonStore {

public:
	SDStore();

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
	char* getCollectionFileName(char* name);
	char* getModelFileName(char* name);
	char* getModelFileName(int id);
	char* getFullPathName(char* collectionFilename, char* modelFilename);
	char modelName[13];
	char collectionName[9];
	char filePath[21];
};

#endif
