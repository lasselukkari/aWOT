#include <SPI.h>
#include <Ethernet.h>
#include <aJSON.h>
#include <ClientInterface.h>
#include <StreamClientAdapter.h>
#include <aJsonStore.h>
#include <HTTPStore.h>

StreamClientAdapter serialClientAdapter(&Serial);
HTTPStore httpStorage(&serialClientAdapter);
aJsonObject* measurementModel  = aJson.parse("{\"name\":\"Sensor 1\", \"value\":0}");



void setup() {
  Serial.begin(9600);
}

void loop(){
  aJson.getObjectItem(measurementModel, "value")->valueint = analogRead(0);
  httpStorage.saveModelToCollection("measurements", measurementModel);
  delay(5000);
}
