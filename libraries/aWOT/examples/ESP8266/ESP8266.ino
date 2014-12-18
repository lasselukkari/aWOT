#include <aWOT.h>
#include <ServerInterface.h>
#include <ESP8266.h>
#include <ESP8266ServerAdapter.h>

char outBuffer[500];
ESP8266 esp(&Serial1, outBuffer);
ESP8266ServerAdapter espServer(&esp);

WebApp app;

// define a handler function
void indexCmd(Request &req, Response &res) {
  // P macro for printing strings from program memory
  P(index1) =
    "<html>\n"
    "<head>\n"
    "<title>Ip Test</title>\n"
    "</head>\n"
    "<body>\n";
    
  P(index2) =
    "\n</body>\n"
    "</body>\n"
    "</html>";

  res.success("text/html");
  res.printP(index1);
  res.print(" ip: ");
  res.print(esp.IP());
  res.print(" ap: ");
  res.print(esp.apIP());
  res.printP(index2);

}

void setup() {
  
  SerialUSB.begin(115200);
  Serial1.begin(115200);
  while(!SerialUSB);
  esp.begin(3);
  esp.connect("SSID", "p4ssw0rd");
  esp.serve(80);
  
  SerialUSB.println(esp.IP()); // SerialUSB for Arduino Due
  SerialUSB.println(esp.apIP()); 

  // mount the handler to the default router
  app.get("", &indexCmd);

}

void loop(){
  // process incoming connections
  app.process(&espServer);
}

