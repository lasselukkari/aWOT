#include <SPI.h>
#include <SD.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <aWOT.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Generate static.bin using awot-scripts and copy it to your SD card
// You also try it out with this file https://github.com/lasselukkari/aWOT/files/5802036/static.bin.zip
#define STATIC_FILES "/static.bin"
#define READ_BUFFER_SIZE 128

WiFiServer server(80);
Application app;
File dataFile;
byte readBuffer[READ_BUFFER_SIZE];

uint32_t calculateHash(const char * string, uint32_t value = 0x811C9DC5) {
  size_t length = strlen(string);

  for (size_t i = 0; i < length; i += 1) {
    value += (value << 1) + (value << 4) + (value << 7) + (value << 8) + (value << 24);
    value ^= string[i];
  }

  return value & 0x7FFFFFFF;
};

uint32_t readNumber(uint32_t address) {
  dataFile.seek(address);
  long four = dataFile.read();
  long three = dataFile.read();
  long two = dataFile.read();
  long one = dataFile.read();

  return ((four << 0) & 0xFF) +
         ((three << 8) & 0xFFFF) +
         ((two << 16) & 0xFFFFFF) +
         ((one << 24) & 0xFFFFFFFF);
}

uint32_t lookupTableIndex(const char * key, uint32_t length) {
  uint32_t hash = calculateHash(key);
  int32_t seed = readNumber(((hash % length) * 4) + 4);

  if (seed < 0) {
    return 0 - seed - 1;
  }

  return calculateHash(key, seed) % length;
};

void fileHandler(Request &req, Response &res) {
  uint32_t fileCount = readNumber(0);
  uint32_t tableIndex = lookupTableIndex(req.path(), fileCount);
  uint32_t infoIndex = 4 + (fileCount * 4) + (tableIndex * 12);
  uint32_t calculatedHash = calculateHash(req.path());
  uint32_t storedHash = readNumber(infoIndex);

  if (calculatedHash != storedHash) {
    return;
  }

  uint32_t offset = readNumber(infoIndex + 4);
  uint32_t length = readNumber(infoIndex + 8);

  dataFile.seek(offset);

  res.set("Connection", "close");
  res.beginHeaders();
  while (length > 0) {
    int toRead = length > READ_BUFFER_SIZE ? READ_BUFFER_SIZE : length;
    dataFile.read(readBuffer, toRead);
    res.write(readBuffer, toRead);
    length = length - toRead;
  }
  res.end();
}

void setup() {
  Serial.begin(9600);

  if (!SD.begin(SS)) {
    Serial.println("SD card initialization failed.");
    while (true) {}
  }

  dataFile = SD.open(STATIC_FILES);
  if (!dataFile) {
    Serial.println("Error opening " STATIC_FILES);
    while (true) {}
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  app.use(&fileHandler);

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
