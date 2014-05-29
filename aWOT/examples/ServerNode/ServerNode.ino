#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>
#include <HTTPServer.h>
#include <ServerInterface.h>
#include <aJsonStore.h>
#include <SDStore.h>
#include <StreamServerAdapter.h>
#include <aJSON.h>

const int chipSelect = 53;
const int slaveSelect = 4;

StreamServerAdapter serialServerAdapter(&Serial);
StreamServerAdapter serial1ServerAdapter(&Serial1);
HTTPServer httpServer("");
SDStore sdStorage;

char * measurementsRoute = "measurements";
char * contentTypeJSON ="application/json";

void getMeasurements(HTTPServer &server, HTTPServer::MethodType, char **, char *, bool) {
  server.httpSuccess(contentTypeJSON);
  sdStorage.streamCollection(measurementsRoute, &Serial1);
}

void postMeasurement(HTTPServer &server, HTTPServer::MethodType, char ** routeParams, char *, bool) {
  aJsonStream serverStream(&server);
  aJsonObject* measurementModel = aJson.parse(&serverStream);
  sdStorage.saveModelToCollection(measurementsRoute, measurementModel); 
  server.httpSuccess(contentTypeJSON);
  aJson.print(measurementModel, &serverStream);
  aJson.deleteItem(measurementModel); 
}

void setup() {
  pinMode(slaveSelect, OUTPUT);
  SD.begin(chipSelect);
  Serial.begin(115200);
  Serial1.begin(115200);
  sdStorage.createCollection("measurements");
  httpServer.onGET(measurementsRoute, &getMeasurements);
  httpServer.onPOST(measurementsRoute, &postMeasurement);
}

void loop(){
  httpServer.processConnection(&serialServerAdapter);
  httpServer.processConnection(&serial1ServerAdapter);
}
