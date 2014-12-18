#include <Arduino.h>
#include "ESP8266.h"


ESP8266::ESP8266(Stream *serial, char * buffer) : buffer(buffer), contentLength(0), gate(0) {
	this->serial = serial;
}

bool ESP8266::begin(byte mode) {

	this->mode = mode;



	this->serial->println("AT+RST");
	delay(ESP8266_RESET_DELAY);
	this->state = BEGUN;
	this->ack = true;

	unsigned long timeOut = millis() + ESP8266_BEGIN_DELAY;

	while (millis() < timeOut){

		if(!ack){
			if(this->serial->find("OK")){
				ack = true;
			} 
		}

		if(state == BEGUN){
			this->serial->println("ATE0");
			state = ECHODISABLED;
			ack = false;
		}

		if(state == ECHODISABLED && ack){
			this->serial->print("AT+CWMODE=");
			this->serial->println(mode);
			state = MODESET;
			ack = false;
		}


		if(state == MODESET && ack){
			this->serial->println("AT+CIPMUX=1");
			state = MUXSET;
			ack = false;
		}

		if(state == MUXSET && ack){
			state = READY;
			return true;	
		}


	}

	return false;


}

bool ESP8266::connect(char * ssid, char * pwd) {

	this->ssid = ssid;
	this->pwd = pwd;

	this->serial->print("AT+CWJAP=\"");
	this->serial->print(ssid);
	this->serial->print("\",\"");
	this->serial->print(pwd);
	this->serial->println("\"");

	unsigned long timeOut = millis() + ESP8266_CONNECT_DELAY;

	while (millis() < timeOut){
		if(this->serial->find("OK")){
			return true;
		}

	}

	return false;

}

bool ESP8266::serve(int port) {

	this->serial->print("AT+CIPSERVER=1,");
	this->serial->println(port);
	this->port = port;

	unsigned long timeOut = millis() + ESP8266_TCP_OPEN_DELAY;

	while (millis() < timeOut){
		if(this->serial->find("OK")){
			return true;
		}

	}

	return false;

}

void ESP8266::reset() {

	bool connected = false;

	while(!connected){

		if(this->begin(this->mode)){
			if (this->connect(this->ssid, this->pwd)){
				if (this->serve(this->port)){
					connected = true;
				}

			}

		}

		delay(5000);

	}


}

bool ESP8266::sendReponse(){

	bool accepted = false;

	this->serial->print("AT+CIPSEND=");
	this->serial->print(this->socket);
	this->serial->print(",");
	this->serial->println(this->contentLength);

	this->contentLength = 0;

	if(!this->serial->find(">")){
		return false;
	}

	this->serial->print(this->buffer);

	if(!this->serial->find("SEND OK")){
		return false;
	}

	this->serial->print("AT+CIPCLOSE=");
	this->serial->println(this->socket);

	if(!this->serial->find("OK")){
		return false;
	}

	return true;


}

char * ESP8266::IP(){

	bool found = false;
	memset(this->ipBuffer, 0, 16);

	this->serial->println("AT+CIFSR");

	unsigned long timeOut = millis() + ESP8266_COMMAND_DELAY;

	while (millis() < timeOut){

		if(this->serial->available()){

			timeOut = millis() + ESP8266_NEXT_CHAR_DELAY;

			int inByte = this->serial->read();

			if (!found && inByte == '+'){

				if (this->serial->find("CIFSR:STAIP,\"")){
					this->serial->readBytesUntil('\"',ipBuffer,15);
					found = true;
				}
			}

		}

	}

	return this->ipBuffer;

}

char * ESP8266::apIP(){

	bool found = false;
	memset(this->ipBuffer, 0, 16);

	this->serial->println("AT+CIFSR");

	unsigned long timeOut = millis() + ESP8266_COMMAND_DELAY;

	while (millis() < timeOut){

		if(this->serial->available()){

			timeOut = millis() + ESP8266_NEXT_CHAR_DELAY;

			int inByte = this->serial->read();

			if (!found && inByte == '+'){

				if (this->serial->find("CIFSR:APIP,\"")){
					this->serial->readBytesUntil('\"',ipBuffer,15);
					found = true;
				}
			}

		}

	}

	return this->ipBuffer;

}

size_t ESP8266::write(uint8_t c){

	this->buffer[contentLength++] = c;
	return 1;

}

size_t ESP8266::write(const char *str){

	int stringLength = strlen(str);

	return this->write((uint8_t *)str, stringLength);

}
size_t ESP8266::write(const uint8_t *buf, size_t size){


	for (int i = 0; i< size; i++){
		this->buffer[this->contentLength++] = *buf++;
	}
	this->buffer[contentLength+1]=0; //todo remove this

	return size;

}

bool ESP8266::connected(){
	return this->available() > 0;
}

int ESP8266::read(){

	char c;

	if(this->available()){

		if(contentLeft--){

			while(!this->serial->available());

			c = this->serial->read();

		}  else {
			c = -1;
		}


	} else {
		c = -1;
	}



	return c;

}

void ESP8266::flush(){


	while(this->serial->available()){
		this->serial->read();
	}

	this->state = READY;

};

void ESP8266::stop(){

	this->buffer[contentLength] = 0;

	if(!sendReponse()){
		this->reset();
	}
	

}

int ESP8266::available(){

	char temp[8];

	if (this->state == READY){


		this->serial->println("AT+CIPREAD");

		if(this->serial->find("OK")){

			unsigned long timeOut = millis() + ESP8266_NEXT_CHAR_DELAY;


			while (this->serial->available() || millis() < timeOut) {


				if(this->serial->available() ){

					timeOut = millis() + ESP8266_NEXT_CHAR_DELAY;

					int inByte = this->serial->read();

					if (inByte == '+'){
						if (this->serial->find("IPD,")){

							memset(temp, 0, 8);
							this->serial->readBytesUntil(',',temp,7);
							this->socket = atoi(temp);
							memset(temp, 0, 8);
							this->serial->readBytesUntil(':',temp,7);
							this->contentLeft = atoi(temp);
							state = READING;

							return this->serial->available();

						}

					} 

				}

			}

		} else {
			this->reset();
		}

		return 0;

	} else if (this->state == READING){

		if (contentLeft > 0){

			unsigned long timeOut = millis() + ESP8266_NEXT_CHAR_DELAY;

			while(!this->serial->available() && timeOut > millis());

			return this->serial->available();

		}


	}	else {

		return 0;

	}

}

int ESP8266::print(const char* str){

	int stringLength = strlen(str);
	strcpy (this->buffer+contentLength, str );
	this->contentLength = this->contentLength + stringLength;
	return stringLength;

}

int ESP8266::println(const char* str){

	int stringLength = strlen(str);
	strcpy ( this->buffer+contentLength, str );
	this->contentLength = this->contentLength + stringLength;
	this->buffer[contentLength++] = '\r';
	this->buffer[contentLength++] = '\n';

	return stringLength + 2;

}

int ESP8266::println(){

	this->buffer[contentLength++] = '\r';
	this->buffer[contentLength++] = '\n';

	return 2;

}

int ESP8266::print(uint16_t b){
	this->buffer[contentLength++] = b;
	return 1;

}

int ESP8266::println(const int number){
	this->buffer[contentLength++] = number;
	this->buffer[contentLength++] = '\r';
	this->buffer[contentLength++] = '\n';
	return 3;

}

int ESP8266::read(uint8_t *buf, size_t size){

	int count = 0;

	while (this->available() && size--){ 

		buf[count++] = this->read();
	}

	contentLeft = contentLeft - count;

	return count;


}

int ESP8266::peek(){

	char c;

	if(this->available()){
		if(contentLeft){

			return this->serial->peek();

		}  else {
			return -1;
		}

	} else {
		return -1;
	}



}






