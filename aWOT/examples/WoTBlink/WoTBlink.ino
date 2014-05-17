#include <SPI.h>
#include <Ethernet.h>
#include <HTTPServer.h>
#include <ServerInterface.h>
#include <SerialServerAdapter.h>
#include <EthernetServerAdapter.h>
#include <aJSON.h>

byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(80);
SerialServerAdapter serialServerAdapter(&Serial);
EthernetServerAdapter ethernetServerAdapter(&server);
HTTPServer httpServer("");

int ledPin = 2; // onboard led (13) can be used with serial but not with ethernet
aJsonObject* ledModel = aJson.parse("{\"id\":\"led\", \"on\":false}");
char * ledRoute = "led";
char * contentTypeJSON ="application/json";

void indexCmd(HTTPServer &server, HTTPServer::MethodType type, char *, bool) {
  server.httpSuccess();
  P(indexHTML) =
    "<html>\n"
    "<head>\n"
    "<title>Led</title>\n"
    "<link href=\"https://netdna.bootstrapcdn.com/bootstrap/3.0.3/css/bootstrap.min.css\" rel=\"stylesheet\">\n"
    "</head>\n"
    "<body>\n"
    "<script data-main=\"https://www.cs.helsinki.fi/u/ljlukkar/led/main\" "
    "src=\"https://cdnjs.cloudflare.com/ajax/libs/require.js/2.1.10/require.min.js\">"
    "</script>\n"
    "</body>\n"
    "</html>";
  server.printP(indexHTML);
}

void getState(HTTPServer &server, HTTPServer::MethodType type, char ** routeParams, char *, bool) {
  aJsonStream serverStream(&server);
  server.httpSuccess(contentTypeJSON);
  aJson.print(ledModel, &serverStream);
}

void setState(HTTPServer &server, HTTPServer::MethodType type, char ** routeParams, char *, bool) {
  aJsonStream serverStream(&server);
  aJson.deleteItem(ledModel);
  ledModel = aJson.parse(&serverStream);
  digitalWrite(ledPin, aJson.getObjectItem(ledModel, "on")->valuebool);
  server.httpSuccess(contentTypeJSON);
  aJson.print(ledModel, &serverStream);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Ethernet.begin(mac);
  Serial.begin(9600);

  httpServer.setDefaultCommand(&indexCmd);
  httpServer.onGET(ledRoute, &getState);
  httpServer.onPUT(ledRoute, &setState);
}

void loop(){
  httpServer.processConnection(&ethernetServerAdapter);
  httpServer.processConnection(&serialServerAdapter);
}
