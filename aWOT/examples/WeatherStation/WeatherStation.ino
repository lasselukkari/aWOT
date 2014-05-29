#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>
#include <HTTPServer.h>
#include <ServerInterface.h>
#include <EthernetServerAdapter.h>
#include <aJSON.h>
#include <DHT.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 20);

EthernetServer server(80);
EthernetServerAdapter ethernetServerAdapter(&server);
HTTPServer httpServer("");

aJsonObject* measurements;
aJsonObject* settings;
unsigned long lastMeasured;

DHT dht;

void indexCmd(HTTPServer &server, HTTPServer::MethodType type, char * tail,
bool) {
  server.httpSuccess();
    P(helloMsg) =
      "<!DOCTYPE html>\n"
      "<html class=\"ui-mobile-rendering\">\n"
      "<head>\n"
      "\t<title>Arduino Weather Station</title>\n"
      "\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
      "\t<link rel=\"stylesheet\" href=\"http://www.cs.helsinki.fi/u/ljlukkar/weatherstation/css/jquery.mobile-1.2.0.css\"/>\n"
      "\t<link rel=\"stylesheet\" href=\"http://www.cs.helsinki.fi/u/ljlukkar/weatherstation/css/jquery-mobile-fluid960.min.css\"/>\n"
      "\t<script src=\"http://www.cs.helsinki.fi/u/ljlukkar/weatherstation/lib/jquery-1.7.1.min.js\" type=\"text/javascript\"></script>\n"
      "\t<script src=\"http://www.cs.helsinki.fi/u/ljlukkar/weatherstation/js/jqm-config.js\" type=\"text/javascript\"></script>\n"
      "\t<script src=\"http://www.cs.helsinki.fi/u/ljlukkar/weatherstation/lib/jquery.mobile-1.2.0.js\" type=\"text/javascript\"></script>\n"
      "\t<script src=\"http://www.cs.helsinki.fi/u/ljlukkar/weatherstation/lib/underscore-min.js\" type=\"text/javascript\"></script>\n"
      "\t<script src=\"http://www.cs.helsinki.fi/u/ljlukkar/weatherstation/lib/backbone-min.js\" type=\"text/javascript\"></script>\n"
      "\t<script src=\"http://www.cs.helsinki.fi/u/ljlukkar/weatherstation/lib/Backbone.ModelBinder.min.js\" type=\"text/javascript\"></script>\n"
      "\t<script src=\"http://www.cs.helsinki.fi/u/ljlukkar/weatherstation/lib/highcharts.js\" type=\"text/javascript\"></script>\n"
      "\t<script src=\"http://www.cs.helsinki.fi/u/ljlukkar/weatherstation/js/main.js\" type=\"text/javascript\"></script>\n"
      "</head>\n"
      "<body class=\"ui-overlay-c\"></body>\n"
      "</html>";
    server.printP(helloMsg);
}

void takeMeasurements() {

  delay(dht.getMinimumSamplingPeriod());
  
  if(strcmp(dht.getStatusString(), "OK") == 0){
    aJson.getObjectItem(measurements, "temperature")->valuefloat = dht.getTemperature();
    aJson.getObjectItem(measurements, "humidity")->valuefloat = dht.getHumidity();
  }
  else{
    aJson.getObjectItem(measurements, "temperature")->valuefloat = random(-300, +300) / 10.0;
    aJson.getObjectItem(measurements, "humidity")->valuefloat = random(1000) / 10.0;
  }

  //add time stamp, model on the front-end doesn't update if none of the values change. rtc would fit here
  aJson.getObjectItem(measurements, "time")->valueint = millis() / 1000;

}


void updateSettings(HTTPServer &server, HTTPServer::MethodType , char ** , char *, bool) {

  aJsonStream serverStream(&server);
  aJsonObject *newSettings = aJson.parse(&serverStream);

  // if model is NULL parsing failed, send error
  if (!newSettings) {
    server.httpServerError();
  } 
  else {
    aJson.deleteItem(settings);
    settings = newSettings;
    server.httpSuccess("application/json");
    aJson.print(settings, &serverStream);
  }

}

void getMeasurements(HTTPServer &server, HTTPServer::MethodType , char ** , char *, bool) {
  aJsonStream serverStream(&server);
  server.httpSuccess("application/json");
  aJson.print(measurements, &serverStream);	
}

void getSettings(HTTPServer &server, HTTPServer::MethodType , char ** , char *, bool) {
  aJsonStream serverStream(&server);
  server.httpSuccess("application/json");
  aJson.print(settings, &serverStream);	
}


void setup() {
  dht.setup(2); // data pin 2
  measurements = aJson.parse("{\"temperature\":0.0, \"humidity\":0.0, \"time\":0}");
  settings = aJson.parse("{\"id\":\"interval\", \"interval\":1}");

  httpServer.onGET("measurements.json", &getMeasurements);
  httpServer.onGET("settings.json", &getSettings);
  httpServer.onPUT("settings.json", &updateSettings);

  httpServer.setDefaultCommand(&indexCmd);

  lastMeasured = 0;

  Serial.begin(9600);
  Ethernet.begin(mac);

  for (byte i = 0; i < 4; i++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[i]);
    if (i < 3) {
      Serial.print('.');
    }
  }
}

// The loop function is called in an endless loop
void loop() {
  httpServer.processConnection(&ethernetServerAdapter);
  if (millis()
    > lastMeasured
    + aJson.getObjectItem(settings, "interval")->valueint
    * 1000) {
    takeMeasurements();
    lastMeasured = millis();
  }
}
