#include <WiFi.h>
#include <aWOT.h>

const char* ssid = "wifi";
const char* password = "pass";

WiFiServer server(80);
Application app;
char authBuffer[200];

void auth(Request &req, Response &res) {
  char * authHeader = req.get("Authorization");

  if (strcmp(authHeader, "Basic c3VwZXI6YWRtaW4=") != 0) { // super:admin in base64
    res.set("WWW-Authenticate", "Basic realm=\"Secret Area\"");
    res.sendStatus(401);
    res.end();
  }
}

void indexCmd(Request &req, Response &res) {
  P(secretPage) =
    "<html>\n"
    "<head>\n"
    "<title>Secret Area</title>\n"
    "</head>\n"
    "<body>\n"
    "<h1>Secret area</h1>\n"
    "</body>\n"
    "</html>";

  res.set("Content-Tpe", "text/html");
  res.printP(secretPage);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println(WiFi.localIP());

  server.begin();

  app.header("Authorization", authBuffer, 200);
  app.use(&auth);
  app.get("/", &indexCmd);
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }
}
