#include <arduino.h>
#include <HTTPStore.h>
#include <aJSON.h>


HTTPStore::HTTPStore(ClientInterface *clientInterface) :
clientInterface(clientInterface) {
	boolean lastConnected = false;
}



void HTTPStore::saveModel(char * name, aJsonObject* model) {

	if (connect()){

		char *body = aJson.print(model);
		if (body != NULL) {

			aJsonObject* idItem = aJson.getObjectItem(model, "id");

			bool isNew = (idItem == NULL) ? true : false;

			if (isNew) {
				printP(httpMethodPost);
			}
			else{
				printP(httpMethodPut);
			}

			
			this->client()->print(name);
			printP (httpVersion);
			this->printCRLF();

			printP (httpContentTypeJSON);
			this->printCRLF();

			printP (httpContentLenght);
			this->client()->print(strlen(body));
			this->printCRLF();

			this->printCRLF();

			this->client()->print(body);

			int status = this->parseStatus();

			while(this->client()->available()){
				this->read();
			}

			free(body);
		}


	}



}

aJsonObject* HTTPStore::findModel(char * name) {
	if (connect()){


		printP (httpMethodGet);
		this->client()->print(name);
		printP (httpVersion);
		this->printCRLF();

		this->printCRLF();

		int status = this->parseStatus();

		if (status >= 200 && status < 300){
			return this->parseBody();
		} else {
			return NULL;
		}
	}

	return NULL;
}

void HTTPStore::removeModel(char * name){
	if (connect()){

		printP (httpMethodDelete);
		this->client()->print(name);
		printP (httpVersion);
		this->printCRLF();

		this->printCRLF();

		int status = this->parseStatus();

		while(this->client()->available()){
			this->read();
		}

	}

}

void HTTPStore::streamModel(char * name, Stream * stream) {

	if (connect()){

		printP (httpMethodGet);
		this->client()->print(name);
		printP (httpVersion);
		this->printCRLF();

		this->printCRLF();

		int status = this->parseStatus();

		while(this->client()->available()){
			stream->write(this->read());
		}

	}
}

void HTTPStore::saveModelStream(char * name, Stream * stream, int contentLenght) {
	if (connect()){
		aJsonStream clientStream(stream);

		printP (httpMethodPost);
		this->client()->print(name);
		printP (httpVersion);
		this->printCRLF();

		printP (httpContentTypeJSON);
		this->printCRLF();

		printP (httpContentLenght);
		this->client()->print(contentLenght);
		this->printCRLF();

		this->printCRLF();

		int status = this->parseStatus();

		while(stream->available()){
			this->client()->print((char)stream->read());
		}


	}

}

void HTTPStore::createCollection(char * name) {
	return; //not supported
}



aJsonObject*  HTTPStore::findCollection(char * name) {

	if (connect()){
		printP (httpMethodGet);
		this->client()->print(name);
		printP (httpVersion);
		this->printCRLF();

		this->printCRLF();

		int status = this->parseStatus();

		return this->parseBody();
	}

	return NULL;

}

void HTTPStore::removeCollection(char * name) {
	if (connect()){

		printP (httpMethodDelete);
		this->client()->print(name);
		printP (httpVersion);
		this->printCRLF();

		this->printCRLF();

		int status = this->parseStatus();

		while(this->client()->available()){
			this->read();
		}

	}
}

void HTTPStore::streamCollection(char * name,  Stream * stream){
	if (connect()){
		printP (httpMethodGet);
		this->client()->print(name);
		printP (httpVersion);
		this->printCRLF();
		this->printCRLF();

		int status = this->parseStatus();

		while(this->client()->available()){
			stream->write(this->read());
		}

	}

}

void HTTPStore::saveModelToCollection(char * name, aJsonObject* model) {
	if (connect()){
		char *body = aJson.print(model);

		if (body != NULL){
			aJsonObject* idItem = aJson.getObjectItem(model, "id");

			bool isNew = (idItem == NULL) ? true : false;

			if (isNew) {
				printP(httpMethodPost);
			}
			else{
				printP(httpMethodPut);
			}

			this->client()->print(name);

			if (!isNew){
				this->client()->print('/');
				this->client()->print(aJson.getObjectItem(model, "id")->valueint);
			}


			printP (httpVersion);
			this->printCRLF();

			printP (httpContentTypeJSON);
			this->printCRLF();

			printP (httpContentLenght);
			this->client()->print(strlen(body));
			this->printCRLF();

			this->printCRLF();

			this->client()->print(body);

			int status = this->parseStatus();


			while(this->client()->available()){
				this->read();
			}

			free(body);
		}


	}

}

aJsonObject* HTTPStore::findModelFromCollection(char * name, int id) {
	if (connect()){

		printP (httpMethodGet);
		this->client()->print(name);
		this->client()->write('/');
		this->client()->print(id);
		printP (httpVersion);
		this->printCRLF();

		this->printCRLF();

		int status = this->parseStatus();

		aJsonObject* model  = this->parseBody();

		return model;
	}

	return NULL;


}

void HTTPStore::removeModelFromCollection(char * name, int id) {
	if (connect()){

		printP (httpMethodDelete);
		this->client()->print(name);
		this->client()->write('/');
		this->client()->print(id);
		printP (httpVersion);
		this->printCRLF();

		this->printCRLF();

		int status = this->parseStatus();

		while(this->client()->available()){
			this->read();
		}

	}
}

void HTTPStore::streamModelFromCollection(char * name, int id, Stream * stream) {
	if (connect()){

		printP (httpMethodGet);
		this->client()->print(name);
		this->client()->write('/');
		this->client()->print(id);
		printP (httpVersion);
		this->printCRLF();

		this->printCRLF();

		int status = this->parseStatus();

		while(this->client()->available()){
			stream->write(this->read());
		}
	}

}



void HTTPStore::saveModelStreamToCollection(char * name, Stream * stream, int contentLenght){

}

void HTTPStore::setServer(IPAddress ip, uint16_t port){
	this->ip = ip;
	this->port = port;
}


bool HTTPStore::connect(){
	if(lastConnected){
		this->client()->stop();
	}
	if (this->client()->connect(this->ip, this->port)){
		lastConnected = true;
		return true;
	}
	else{
		lastConnected = false;
		return 	false;
	}

}

size_t HTTPStore::write(uint8_t ch){
	return this->client()->write(ch);
}
int HTTPStore::available(){
	return this->client()->available();
}

int HTTPStore::read(){
	unsigned long timeoutTime = millis() + 1000;

	while (this->client()->connected()) {

		int ch = this->client()->read();

		if (ch != -1) {
			return ch;
		} else {
			unsigned long now = millis();
			if (now > timeoutTime) {
				return -1;
			}
		}
	}
	return -1;
}

int HTTPStore::peek(){
	return this->client()->peek();
}
void HTTPStore::flush(){
	return this->client()->flush();
}


int HTTPStore::parseStatus(){
	char statusBuffer[4];
	long start = millis();
	while (!this->client()->available()){
		if(millis()-start > 60000){
			//timeout
			return 0;
		}
	}
	byte i = 0;
	bool skipping = true;
	while (this->client()->available()){
		if(skipping){
			while ((char)this->read() != ' '){
				skipping = true;
			}
			skipping = false;
		}
		else{
			char c = (char)this->read();

			if (c ==' '){
				break;
			}
			statusBuffer[i++] = c;
		}

	}


	bool current_line_is_blank = false;

	while (this->client()->available()){
		char c = this->read();

		if (c == '\n' && current_line_is_blank) {
			break;
		}
		if (c == '\n') {
			// we're starting a new line
			current_line_is_blank = true;
		} else if (c != '\r') {
			// we've gotten a character on the current line
			current_line_is_blank = false;
		}
	}

	statusBuffer[i] = '\0';

	return atoi(statusBuffer);

}

aJsonObject * HTTPStore::parseBody(){

	aJsonStream bodyStream(this);
	return aJson.parse(&bodyStream);

}


void HTTPStore::printP(const prog_uchar *str) {
	// copy data out of program memory into local storage, write out in
	// chunks of 32 bytes to avoid extra short TCP/IP packets
	uint8_t buffer[32];
	size_t bufferEnd = 0;

	while (buffer[bufferEnd++] = pgm_read_byte(str++)) {
		if (bufferEnd == 32) {
			client()->write(buffer, 32);
			bufferEnd = 0;
		}
	}

	// write out everything left but trailing NUL
	if (bufferEnd > 1)
		client()->write(buffer, bufferEnd - 1);
}


void HTTPStore::printCRLF() {
	client()->write((const uint8_t *) "\r\n", 2);
}
