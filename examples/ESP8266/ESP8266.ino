#include <aWOT.h>
#include <ESP8266.h> // use ESP8266Server fork https://github.com/lasselukkari/arduino-ESP8266
#include <ESP8266Server.h> 

ESP8266 wifi = ESP8266(Serial1);
ESP8266Server server(wifi, 80);
IPAddress ip;

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
  res.print("ip: ");
  res.print(ip);
  res.printP(index2);
}

void setup() {

  Serial.begin(115200);
  Serial1.begin(115200);

  wifi.begin();
  wifi.setMode(ESP8266_WIFI_BOTH);
  wifi.setMultipleConnections(true);
  wifi.joinAP("ssid", "p4ssw0rd");

  wifi.getIP(ESP8266_WIFI_STATION, ip);
  Serial.println(ip);

  server.begin();

  // mount the handler to the default router
  app.get("", &indexCmd);

}

void loop(){
  ESP8266Client * client = server.available();
  if(client){
    app.process(client);
  }
}

