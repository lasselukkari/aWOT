#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <HTTPServer.h>
#include <ServerInterface.h>
#include <StreamServerAdapter.h>
#include <EthernetServerAdapter.h>
#include <aJSON.h>
#include <aJsonStore.h>
#include <MemoryStore.h>
#include <RCSwitch.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 20);
EthernetServer server(80);
StreamServerAdapter serialServerAdapter(&Serial);
EthernetServerAdapter ethernetServerAdapter(&server);

HTTPServer httpServer("");
MemoryStore memoryStore;
RCSwitch mySwitch = RCSwitch();

// use dictionary to save RAM with key names
char *dictionary[] = {"id", "name", "state", "group", "number", NULL };

//send HTML that loads the external javascript and css files for the UI
void indexCmd(HTTPServer &server, HTTPServer::MethodType type, char *, bool) {
  server.httpSuccess();
    P(helloMsg) =
      "<!DOCTYPE html>\n"
      "<html lang=\"en\">\n"
      "<head>\n"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
      "<title>RC-Switch</title>\n"
      "<link href=\"https://netdna.bootstrapcdn.com/bootstrap/3.0.3/css/bootstrap.min.css\" rel=\"stylesheet\">\n"
      "<!--[if lt IE 9]>\n <script src=\"https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js\"></script>\n "
      "<script src=\"https://oss.maxcdn.com/libs/respond.js/1.3.0/respond.min.js\"></script>\n <![endif]-->\n"
      "</head>\n"
      "<body style=\"padding-top: 70px\">\n<"
      "script data-main=\"https://www.cs.helsinki.fi/u/ljlukkar/rc-switch/main\" "
      "src=\"https://cdnjs.cloudflare.com/ajax/libs/require.js/2.1.10/require.min.js\">"
      "</script>\n"
      "</body>\n"
      "</html>";
    server.printP(helloMsg);
}

// same command can be used for adding and updating devices
void upsertModel(HTTPServer &server, HTTPServer::MethodType type,
char ** routeParams, char *, bool) {

  // parse the object as a stream
  aJsonStream serverStream(&server);

  // the first URL parameter holds the collection name
  // in this case it's "devices"
  char * collentionName = routeParams[0];

  aJsonObject *model = aJson.parse(&serverStream);

  // if model is NULL parsing failed, send error
  if (!model) {

    server.httpServerError();
  } 
  else {
    memoryStore.saveModelToCollection(collentionName, model);

    if (type == HTTPServer::POST) {
      server.httpCreated("application/json");
    } 
    else { // HTTP PUT
      server.httpSuccess("application/json");
    }

    // send the result after the headers
    aJson.print(model, &serverStream);

    // close the connection so the request doesn't hold
    server.close();

    // set the state of the switch
    bool switchOn = aJson.getObjectItem(model, "state")->valuebool;
    int group = atoi(aJson.getObjectItem(model, "group")->valuestring);
    int number = atoi(aJson.getObjectItem(model, "number")->valuestring);

    if (switchOn) {
      mySwitch.switchOn(group, number);
    } 
    else {
      mySwitch.switchOff(group, number);
    }

  }

}

void deleteModel(HTTPServer &server, HTTPServer::MethodType, char ** routeParams, char *, bool) {
  char * collentionName = routeParams[0];
  int id = atoi(routeParams[1]);
  memoryStore.removeModelFromCollection(collentionName, id);
  server.httpNoContent();
}

void getCollection(HTTPServer &server, HTTPServer::MethodType, char ** routeParams, char *, bool) {
  char * collentionName = routeParams[0];
  server.httpSuccess("application/json");
  memoryStore.streamCollection(collentionName, &server);
}

void setup() {
  Serial.begin(115200);

  mySwitch.enableTransmit(2);

  if (Ethernet.begin(mac)) {
  	Serial.print("My IP address: ");
  	for (byte thisByte = 0; thisByte < 4; thisByte++) {
  		// print the value of each byte of the IP address:
  		Serial.print(Ethernet.localIP()[thisByte], DEC);
  		Serial.print(".");
  	}
  	Serial.println();
  }

  // parts beginning with ':' are treated as wild cards and parsed along with the other route parameters

  httpServer.onGET("devices", &getCollection);
  httpServer.onPOST("devices", &upsertModel);
  httpServer.onPUT("devices/:id", &upsertModel);
  httpServer.onDELETE("devices/:id", &deleteModel);

  httpServer.setDefaultCommand(&indexCmd);
  aJson.setDictionary(dictionary);
  memoryStore.createCollection("devices");

}

void loop() {
  httpServer.processConnection(&serialServerAdapter);
  httpServer.processConnection(&ethernetServerAdapter);
}


