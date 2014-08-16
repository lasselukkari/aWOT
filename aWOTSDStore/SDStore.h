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
	char* getCollectionFileName(char* name);
	char* getModelFileName(char* name);
	char* getModelFileName(int id);
	char* getFullPathName(char* collectionFilename, char* modelFilename);
	char modelName[13];
	char collectionName[9];
	char filePath[21];
};

#endif
