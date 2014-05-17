#include <aJSON.h>
#include <ClientInterface.h>
#include <SerialClientAdapter.h>
#include <aJsonStore.h>
#include <HTTPStore.h>


// EthernetClient client;
// EthernetClientAdapter ethernetClientAdapter(client);
// byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// byte blinkServer[] = { 192, 168, 0, 11 };
// HTTPStore httpStorage(&ethernetClientAdapter);

SerialClientAdapter serialClientAdapter(&Serial);
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
