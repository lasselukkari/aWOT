#include <arduino.h>
#include <MemoryStore.h>
#include <aJSON.h>

MemoryStore::MemoryStore() {
	this->root = aJson.createObject();
	this->nextId = 1;
}

void MemoryStore::saveModel(char * name, aJsonObject* model) {
	aJson.addItemToObject(root, name, model);
}

aJsonObject* MemoryStore::findModel(char * name) {
	return aJson.getObjectItem(root, name);
}

void MemoryStore::removeModel(char * name){
	aJson.deleteItemFromObject(root, name);
}

void MemoryStore::streamModel(char * name, Stream * stream) {
	aJsonStream aJsonStream(stream);
	aJson.print(aJson.getObjectItem(root, name), &aJsonStream);
}

void MemoryStore::saveModelStream(char * name, Stream * stream, int contentLenght) {
	aJsonStream aJsonStream(stream);
	this->saveModel(name, aJson.parse(&aJsonStream));
}

void MemoryStore::createCollection(char * name) {
	aJsonObject* modelArray = aJson.createArray();
	aJson.addItemToObject(root, name, modelArray);
}



aJsonObject*  MemoryStore::findCollection(char * name) {
	return aJson.getObjectItem(root, name);
}

void MemoryStore::removeCollection(char * name) {
	aJson.deleteItemFromObject(root, name);
}

void MemoryStore::streamCollection(char * name,  Stream * stream){
	aJsonStream aJsonStream(stream);
	aJson.print(aJson.getObjectItem(root, name), &aJsonStream);
}

void MemoryStore::saveModelToCollection(char * name, aJsonObject* model) {

	aJsonObject *collection = aJson.getObjectItem(root, name);
	aJsonObject* idItem = aJson.getObjectItem(model, "id");

	bool isNew = (idItem == NULL) ? true : false;

	if (isNew) {

		aJson.addNumberToObject(model, "id", this->nextId++);
		aJson.addItemToArray(collection, model);

	} else {

		int id = aJson.getObjectItem(model, "id")->valueint;
		int modelIndex = 0;

		aJsonObject * oldModel = collection->child;

		while (oldModel && aJson.getObjectItem(oldModel, "id")->valueint != id) {
			oldModel = oldModel->next;
			modelIndex++;
		}

		if (oldModel != model) {
			aJson.replaceItemInArray(collection, modelIndex, model);
		}
	}
}

aJsonObject* MemoryStore::findModelFromCollection(char * name, int id) {

	aJsonObject *collection = aJson.getObjectItem(root, name);

	aJsonObject * model = collection->child;
	while (model && aJson.getObjectItem(model, "id")->valueint != id) {
		model = model->next;
	}
	return model;
}

void MemoryStore::removeModelFromCollection(char * name, int id) {

	aJsonObject *collection = aJson.getObjectItem(root, name);
	int modelIndex = 0;

	aJsonObject * model = collection->child;

	while (model && aJson.getObjectItem(model, "id")->valueint != id) {
		model = model->next;
		modelIndex++;
	}

	// todo don't delete if not found
	aJson.deleteItemFromArray(collection, modelIndex);
}

void MemoryStore::streamModelFromCollection(char * name, int id, Stream * stream) {
	aJsonStream aJsonStream(stream);
	aJson.print(this->findModelFromCollection(name, id), &aJsonStream);
}



void MemoryStore::saveModelStreamToCollection(char * name, Stream * stream, int contentLenght){
	aJsonStream aJsonStream(stream);
	this->saveModel(name, aJson.parse(&aJsonStream));
}





