#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>
#include <HTTPServer.h>
#include <ServerInterface.h>
#include <SerialServerAdapter.h>
#include <EthernetServerAdapter.h>
#include <aJSON.h>
#include <aJsonStore.h>
#include <MemoryStore.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 20);
EthernetServer server(80);
EthernetServerAdapter ethernetServerAdapter(&server);

HTTPServer httpServer("");
MemoryStore memoryStore;

// use dictionary to save RAM with key names
char *dictionary[] = {
  "id", "name", NULL };

//send HTML that loads the external javascript and css files for the UI
void indexCmd(HTTPServer &server, HTTPServer::MethodType type, char *, bool) {
  server.httpSuccess();
  P(helloMsg) =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    " <meta charset=\"utf-8\">\n"
    " <title>Example test</title>\n"
    " <link rel=\"stylesheet\" href=\"http://code.jquery.com/qunit/qunit-1.14.0.css\">\n"
    "</head>\n"
    "<body>\n"
    "<div id=\"qunit\"></div>\n"
    "<div id=\"qunit-fixture\"></div>\n"
    "<script src=\"http://code.jquery.com/jquery-1.11.0.min.js\"></script>\n"
    "<script src=\"http://code.jquery.com/qunit/qunit-1.14.0.js\"></script>\n"
    "<script src=\"http://www.cs.helsinki.fi/u/ljlukkar/qunit/tests.js\"></script>\n"
    "</body>\n"
    "</html>";
  server.printP(helloMsg);
}

// same command can be used for adding and updating devices
void upsertModel(HTTPServer &server, HTTPServer::MethodType type, char ** routeParams, char *, bool) {
  aJsonStream serverStream(&server);
  char * collentionName = routeParams[0];
  aJsonObject *model = aJson.parse(&serverStream);

  if (!model) {
    server.httpServerError();
  } 
  else {
    memoryStore.saveModelToCollection(collentionName, model);

    if (type == HTTPServer::POST) {
      server.httpCreated("application/json");
    } 
    else {
      server.httpSuccess("application/json");
    }
    aJson.print(model, &serverStream);
  }
}


void getModel(HTTPServer &server, HTTPServer::MethodType, char ** routeParams, char *, bool) {
  char * collentionName = routeParams[0];
  int id = atoi(routeParams[1]);
  aJsonObject *model = memoryStore.findModelFromCollection(collentionName, id);
  
  if (!model) {
    server.httpNotFound();
  }
  else{
    aJsonStream serverStream(&server);
    server.httpSuccess("application/json");
    aJson.print(model, &serverStream);
  } 
}

void deleteModel(HTTPServer &server, HTTPServer::MethodType, char ** routeParams, char *, bool) {
  char * collentionName = routeParams[0];
  int id = atoi(routeParams[1]);
  aJsonObject *model = memoryStore.findModelFromCollection(collentionName, id);
  
  if(!model){
    server.httpNotFound();
  }
  else{
    memoryStore.removeModelFromCollection(collentionName, id);
    server.httpNoContent();
  }
}

void getCollection(HTTPServer &server, HTTPServer::MethodType, char ** routeParams, char *, bool) {
  char * collentionName = routeParams[0];
  server.httpSuccess("application/json");
  memoryStore.streamCollection(collentionName, &server);
}

void setup() {
  Serial.begin(115200);

  if (Ethernet.begin(mac)) {
    Serial.print("My IP address: ");
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
      Serial.print(Ethernet.localIP()[thisByte], DEC);
      Serial.print(".");
    }
    Serial.println();
  }
  else{
    Serial.println("Ethernet failed");
  }

  httpServer.onGET("examples", &getCollection);
  httpServer.onPOST("examples", &upsertModel);
  httpServer.onGET("examples/:id", &getModel);
  httpServer.onPUT("examples/:id", &upsertModel);
  httpServer.onDELETE("examples/:id", &deleteModel);

  httpServer.setDefaultCommand(&indexCmd);
  aJson.setDictionary(dictionary);
  memoryStore.createCollection("examples");

}

void loop() {
  httpServer.processConnection(&ethernetServerAdapter);
}
