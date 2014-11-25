#include <arduino.h>
#include <SDStore.h>
#include <aJSON.h>
#include <SD.h>

SDStore::SDStore() {

}

void SDStore::saveModel(char * name, aJsonObject* model) {

	char * modelFileName = this->getModelFileName(name);

	if (SD.exists(modelFileName)) {
		SD.remove(modelFileName);
	}

	File modelFile = SD.open(modelFileName, FILE_WRITE);
	if (modelFile) {
		aJsonStream fileStream(&modelFile);
		aJson.print(model, &fileStream);
		modelFile.close();
	}

}

aJsonObject* SDStore::findModel(char * name) {

	char * modelFileName = this->getModelFileName(name);

	if (SD.exists(modelFileName)) {
		File modelFile = SD.open(modelFileName);
		aJsonStream fileStream(&modelFile);
		aJsonObject* model = aJson.parse(&fileStream);
		modelFile.close();
		return model;
	} else {
		return NULL;
	}

}

void SDStore::removeModel(char * name) {

	char * modelFileName = this->getModelFileName(name);

	if (SD.exists(modelFileName)) {
		SD.remove(modelFileName);
	}

}

void SDStore::findModel(char * name, Stream * stream) {
	aJsonStream aJsonStream(stream);

	char * modelFileName = this->getModelFileName(name);

	File modelFile = SD.open(modelFileName);

	while (modelFile.available()) {
		aJsonStream.print((char) modelFile.read());
	}

	modelFile.close();

}

void SDStore::saveModel(char * name, Stream * stream, int contentLenght) {
	aJsonStream aJsonStream(stream);
	aJsonObject* model = aJson.parse(&aJsonStream);
	this->saveModel(name, model);
	aJson.deleteItem(model);
}

void SDStore::createCollection(char * name) {
	char * collectionName = getCollectionFileName(name);
	if (!SD.exists(collectionName)) {
		SD.mkdir(collectionName);

		char * optionsFileName = this->getModelFileName("colops");

		char * optionsPath = this->getFullPathName(collectionName,
				optionsFileName);

		File colOpsFile = SD.open(optionsPath, FILE_WRITE);

		if (colOpsFile) {
			aJsonStream fileStream(&colOpsFile);
			aJsonObject *colOpsModel = aJson.createObject();
			aJson.addNumberToObject(colOpsModel, "nextId", 1);

			colOpsFile.seek(0);
			aJson.print(colOpsModel, &fileStream);
			colOpsFile.close();
			aJson.deleteItem(colOpsModel);
		}
	}
}

aJsonObject* SDStore::findCollection(char * name) {
	aJsonObject* collection = NULL;

	char * collectionName = getCollectionFileName(name);

	if (SD.exists(collectionName)) {
		collection = aJson.createArray();

		File collectionFile = SD.open(collectionName);
		File modelFile = collectionFile.openNextFile();
		char * optionsFileName = this->getModelFileName("colops");

		while (modelFile) {

			if (strcasecmp(optionsFileName, modelFile.name()) != 0) {
				aJsonStream fileStream(&modelFile);
				aJsonObject* model = aJson.parse(&fileStream);
				aJson.addItemToArray(collection, model);
			}

			modelFile.close();
			modelFile = collectionFile.openNextFile();
		}
		collectionFile.close();

	}

	return collection;

}

void SDStore::removeCollection(char * name) {

	char * collectionFilename = this->getCollectionFileName(name);

	if (SD.exists(collectionFilename)) {

		File collectionFile = SD.open(collectionFilename);

		File modelFile = collectionFile.openNextFile();

		while (modelFile) {
			char * fullFilePath = this->getFullPathName(collectionFilename,
					modelFile.name());
			SD.remove(fullFilePath);
			modelFile.close();
			modelFile = collectionFile.openNextFile();
		}
		collectionFile.close();
		SD.rmdir(name);
	}

}

void SDStore::findCollection(char * name, Stream * stream) {
	aJsonStream aJsonStream(stream);

	char * collectionName = getCollectionFileName(name);
	boolean firstModel = true;

	if (SD.exists(collectionName)) {

		File collectionFile = SD.open(collectionName);
		File modelFile = collectionFile.openNextFile();
		char * optionsFileName = this->getModelFileName("colops");

		aJsonStream.print('[');

		while (modelFile) {

			if (strcasecmp(optionsFileName, modelFile.name()) != 0) {
				while (modelFile.available()) {
					aJsonStream.print((char) modelFile.read());
				}
			}

			modelFile.close();

			modelFile = collectionFile.openNextFile();

			if (modelFile && !firstModel) {
				aJsonStream.print(',');
			}
			
			firstModel = false;
		}

		aJsonStream.print(']');
		collectionFile.close();
	}

}

void SDStore::saveToCollection(char * name, aJsonObject* model) {

	aJsonObject* idItem = aJson.getObjectItem(model, "id");

	bool isNew = (idItem == NULL) ? true : false;
	if (isNew) {
		char * collectionFileName = this->getCollectionFileName(name);
		char * optionsFileName = this->getModelFileName("colops");

		char * optionsPath = this->getFullPathName(collectionFileName,optionsFileName);

		if (SD.exists(optionsPath)) {
			File colOpsFile = SD.open(optionsPath, FILE_WRITE);
			colOpsFile.seek(0);
			aJsonStream fileStream(&colOpsFile);
			aJsonObject * colOpsModel = aJson.parse(&fileStream);
			int nextId = aJson.getObjectItem(colOpsModel, "nextId")->valueint++;
			aJson.addNumberToObject(model, "id", nextId);
			colOpsFile.seek(0);
			aJson.print(colOpsModel, &fileStream);
			aJson.deleteItem(colOpsModel);
			colOpsFile.close();

		}

	}

	char * collectionFileName = this->getCollectionFileName(name);
	char * modelFileName = this->getModelFileName(aJson.getObjectItem(model, "id")->valueint);

	File collection = SD.open(collectionFileName);

	if (collection.isDirectory()) {
		collection.close();

		char * fullPath = this->getFullPathName(collectionFileName,
				modelFileName);

		if (SD.exists(fullPath)) {
			SD.remove(fullPath);
		}

		File modelFile = SD.open(fullPath, FILE_WRITE);
		if (modelFile) {
			aJsonStream fileStream(&modelFile);
			aJson.print(model, &fileStream);
			modelFile.close();
		}
	}
}

aJsonObject* SDStore::findFromCollection(char * name, int id) {

	char * collectionFileName = this->getCollectionFileName(name);
	char * modelFileName = this->getModelFileName(id);

	char * fullPath = this->getFullPathName(collectionFileName, modelFileName);

	if (SD.exists(fullPath)) {
		File modelFile = SD.open(fullPath);
		aJsonStream fileStream(&modelFile);
		aJsonObject * model = aJson.parse(&fileStream);
		modelFile.close();
		return model;
	} else {
		return NULL;
	}

}

void SDStore::removeFromCollection(char * name, int id) {

	char * collectionFileName = this->getCollectionFileName(name);

	File collection = SD.open(collectionFileName);
	if (collection.isDirectory()) {
		collection.close();
		char * modelFileName = this->getModelFileName(id);
		char * fullFilePath = this->getFullPathName(collectionFileName,
				modelFileName);
		if (SD.exists(fullFilePath)) {
			SD.remove(fullFilePath);
		}
	}
}

void SDStore::findFromCollection(char * name, int id, Stream * stream) {
	aJsonStream aJsonStream(stream);

	char * collectionFileName = this->getCollectionFileName(name);
	char * modelFileName = this->getModelFileName(id);

	char * fullPath = this->getFullPathName(collectionFileName, modelFileName);

	File modelFile = SD.open(fullPath);

	while (modelFile.available()) {
		aJsonStream.print((char) modelFile.read());
	}

	modelFile.close();
}

void SDStore::saveToCollection(char * name, Stream * stream,
		int contentLenght) {
	aJsonStream aJsonStream(stream);
	aJsonObject* model = aJson.parse(&aJsonStream);
	this->saveToCollection(name, model);
	aJson.deleteItem(model);
}

char * SDStore::getCollectionFileName(char * name) {
	memset(collectionName, '\0', 9);
	strncpy(collectionName, name, 8);
	return collectionName;
}

char * SDStore::getModelFileName(char * name) {

	memset(modelName, '\0', 8);
	int nameLenght = strlen(name);
	if (nameLenght > 8) {
		strncpy(modelName, name, 8);
		nameLenght = 8;
	} else {
		strcpy(modelName, name);
	}

	strcpy(modelName + nameLenght, ".jsn");
	return modelName;

}

char * SDStore::getModelFileName(int id) {
	char itoaBuffer[9];

	itoa(id, itoaBuffer, 10);
	int charCount = strlen(itoaBuffer);

	memset(modelName, '0', 8);
	strncpy(modelName + (8 - charCount), itoaBuffer, charCount);
	strcpy(modelName + 8, ".jsn");

	return modelName;

}

char * SDStore::getFullPathName(char* collectionFileName, char* modelFileName) {

	int collectionNameLenght = strlen(collectionFileName);
	strcpy(this->filePath, collectionFileName);
	this->filePath[collectionNameLenght] = '/';
	strcpy(this->filePath + collectionNameLenght + 1, modelFileName);
	return this->filePath;

}
