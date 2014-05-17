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
aJsonObject* ledModel  = aJson.parse("{\"id\":\"led\", \"on\":true}");

int buttonPin = 2;
bool lastState = 0;

void setup() {
  //Ethernet.begin(mac);
  //httpStorage.setServer(server, 80);
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
}

void loop(){
  bool buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && buttonState != lastState) {
    bool oldState = aJson.getObjectItem(ledModel, "on")->valuebool;
    aJson.getObjectItem(ledModel, "on")->valuebool = !oldState;
    httpStorage.saveModel("led", ledModel);
  }
  lastState = buttonState;
}
