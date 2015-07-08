#include <ESP8266WiFi.h>
#include <aWOT.h>

const char* ssid = "ssid";
const char* password = "pass";
WiFiServer server(80);
WebApp app;

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
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  // mount the handler to the default router
  app.get("/example", &indexCmd);

}

void loop(){
  WiFiClient client = server.available();
  
  if (client.available()){
      app.process(&client);
  }

  delay(5);
}

