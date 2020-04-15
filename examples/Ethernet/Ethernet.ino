#include <SPI.h>
#include <Ethernet.h>
#include <aWOT.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(80);

Application app;

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

  res.set("Content-Type", "text/html");
  res.printP(index);

}

void overwritedNotFound(Request &req, Response &res){

    res.status(404);
    res.set("Content-Type", "application/json");
    res.print(F("{\"error\":\"route not found\"}"));

    Serial.print("Route: ");
    Serial.print(req.path());
    Serial.print(" error 404 'Not Found' from client: ");
    Serial.println(((EthernetClient*)req.stream())->remoteIP());

}

void setup() {

  Serial.begin(115200);

  if (Ethernet.begin(mac)) {
    Serial.println(Ethernet.localIP());
  } else{
    Serial.println("Ethernet failed");
  }

  // mount the handler to the default router
  app.get("/", &indexCmd);
}

void loop(){
  EthernetClient client = server.available();
  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
