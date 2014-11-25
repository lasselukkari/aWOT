#include <SPI.h>
#include <Ethernet.h>
#include <aWOT.h>
#include <ServerInterface.h>
#include <EthernetServerAdapter.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(80);
EthernetServerAdapter ethernetServer(&server);

WebApp app;

// define a handler function
void indexCmd(Request &req, Response &res) {

  // P macro for printing strings from program memory
  P(index) =
    "<html>\n"
    "<head>\n"
    "<title>Hello World!</title>\n"
    "</head>\n"
    "<body>\n"
    "<h1>Greetings middle earth!</h1>\n"
    "</body>\n"
    "</html>";

  res.success("text/html");
  res.printP(index);

}

void setup() {

  Serial.begin(115200);

  if (Ethernet.begin(mac)) {
    Serial.println(Ethernet.localIP());
  } else{
    Serial.println("Ethernet failed");
  }

  // mount the handler to the default router
  app.get("", &indexCmd);

}

void loop(){
  // process incoming connections
  app.process(&ethernetServer);
}
