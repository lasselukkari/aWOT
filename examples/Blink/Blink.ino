#include <SPI.h>
#include <Ethernet.h>
#include <aWOT.h>
#include <StreamClient.h>
#include <aJSON.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(80);
StreamClient serialClient(Serial); 
WebApp app;

int ledPin = 2; // onboard led (13) can be used with serial but not with ethernet
aJsonObject* ledModel = aJson.parse("{\"id\":\"led\", \"on\":false}");

void indexCmd(Request &req, Response &res) {

  P(index) =
    "<html>\n"
    "<head>\n"
    "<title>Led</title>\n"
    "<link href=\"https://netdna.bootstrapcdn.com/bootstrap/3.0.3/css/bootstrap.min.css\" rel=\"stylesheet\">\n"
    "</head>\n"
    "<body>\n"
    "<script data-main=\"http://www.awot.net/led/main\" "
    "src=\"https://cdnjs.cloudflare.com/ajax/libs/require.js/2.1.10/require.min.js\">"
    "</script>\n"
    "</body>\n"
    "</html>";

  res.success("text/html");
  res.printP(index);

}

void getState(Request &req, Response &res) {

  aJsonStream outStream(&res);

  res.success();
  aJson.print(ledModel, &outStream);

}

void setState(Request &req, Response &res) {

  aJsonStream inStream(&req);
  aJsonStream outStream(&res);

  aJson.deleteItem(ledModel);

  ledModel = aJson.parse(&inStream);

  bool newState = aJson.getObjectItem(ledModel, "on")->valuebool;

  digitalWrite(ledPin, newState);

  res.success();
  aJson.print(ledModel, &outStream);

}

void setup() {

  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);

  if (Ethernet.begin(mac)) {
    Serial.println(Ethernet.localIP());
  }
  else{
    Serial.println("Ethernet failed");
  }

  app.get("", &indexCmd);
  app.get("led", &getState);
  app.put("led", &setState);

}

void loop(){
  
  EthernetClient client = server.available();
  if (client){
    app.process(&client);
  }
  
  if (serialClient.available()){
    app.process(&serialClient);
  }

}