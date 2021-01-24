#include <WiFi.h>
#include <aWOT.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

WiFiServer server(80);
Application app;
char authBuffer[200];

// https://nachtimwald.com/2017/04/02/constant-time-string-comparison-in-c/
bool str_iseq(const char *s1, const char *s2) {
  int    m = 0;
  size_t i = 0;
  size_t j = 0;
  size_t k = 0;

  if (s1 == NULL || s2 == NULL) {
    return false;
  }

  while (true) {
    m |= s1[i] ^ s2[j];

    if (s1[i] == '\0') {
      break;
    }

    i++;

    if (s2[j] != '\0') {
      j++;
    }

    if (s2[j] == '\0') {
      k++;
    }
  }

  return m == 0;
}

void auth(Request &req, Response &res) {
  char * authHeader = req.get("Authorization");

  if (!str_iseq(authHeader, "Basic c3VwZXI6YWRtaW4=")) { // super:admin in base64
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

  res.set("Content-Type", "text/html");
  res.printP(secretPage);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
    client.stop();
  }
}
