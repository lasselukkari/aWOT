#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

// Manually add "#define STD_FUNCTION_MIDDLEWARE" to the top of the aWOT.h file
#include <aWOT.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

WiFiServer server(80);
Application app;

class Counter {
  public:
    Counter(int initialCount) {
      _count = initialCount;
    }

    void handleRequest(Request &req, Response &res) {
      _count++;
      res.print("I have been called ");
      res.print(_count);
      res.print(" times!");
    }

  private:
    int _count = 0;
};

Counter counter(666);

void callback(Request &req, Response &res) {
  res.print("Hello World!");
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());


  app.get("/", std::bind(&Counter::handleRequest, &counter, std::placeholders::_1, std::placeholders::_2));

  // Do not add '&' in front of the parameter when STD_FUNCTION_MIDDLEWARE has been defined.
  app.get("/callback", callback);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
