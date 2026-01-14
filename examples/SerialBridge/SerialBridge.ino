/*
  SerialBridge.ino - Serial to WebSocket bridge for Arduino UNO + Ethernet
  Shield

  This example demonstrates the Stream-based WebSocket API by creating a
  bidirectional bridge between Serial and WebSocket clients. Uses minimal
  RAM suitable for Arduino UNO.

  Hardware: Arduino UNO + Ethernet Shield (W5100/W5500)
*/

#include <Ethernet.h>
#include <aWOT.h>

// MAC address for Ethernet shield
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetServer server(80);
App app;
awot::WebSocket ws;

// Simple HTML page in PROGMEM
P(indexHtml) =
    "<!DOCTYPE html><html><head><title>Serial Bridge</title>"
    "<style>body{font-family:monospace;background:#1a1a2e;color:#0f0;padding:"
    "20px}"
    "#t{background:#000;padding:10px;height:300px;overflow-y:auto;white-space:"
    "pre-wrap;border:1px solid #0f0}"
    "input{background:#000;color:#0f0;border:1px solid "
    "#0f0;padding:5px;width:70%}"
    "button{background:#0f0;color:#000;border:none;padding:5px "
    "10px}</style></head>"
    "<body><h2>Serial Bridge</h2><div id='t'></div>"
    "<input id='i' onkeypress=\"if(event.key==='Enter')s()\">"
    "<button onclick='s()'>Send</button>"
    "<script>var t=document.getElementById('t'),i=document.getElementById('i'),"
    "w=new WebSocket('ws://'+location.host+'/ws');"
    "w.onmessage=function(e){t.textContent+=e.data;t.scrollTop=t.scrollHeight};"
    "w.onopen=function(){t.textContent='Connected\\n'};"
    "function "
    "s(){if(i.value){w.send(i.value);i.value=''}}</script></body></html>";

void indexHandler(Request& req, Response& res) {
  res.set("Content-Type", "text/html");
  res.printP(indexHtml);
}

void wsHandler(Request& req, Response& res) {
  char key[32] = {0};
  char* keyHeader = req.get("Sec-WebSocket-Key");
  if (keyHeader) {
    strncpy(key, keyHeader, sizeof(key) - 1);
  }

  if (ws.upgrade(req.stream(), key)) {
    res.keepOpen();
  } else {
    res.sendStatus(400);
  }
}

// WebSocket message handler - using Stream interface
void onMessage(awot::WebSocketMessage& msg) {
  // Read message and forward to Serial
  while (msg.available()) {
    int byte = msg.read();
    if (byte >= 0) {
      Serial.write(byte);
    }
  }
  Serial.println();  // Add newline

  // Echo confirmation back to sender
  msg.print("> OK\n");
}

void onConnect(int clientId) {
  Serial.print(F("WS client connected: "));
  Serial.println(clientId);
}

void onDisconnect(int clientId) {
  Serial.print(F("WS client disconnected: "));
  Serial.println(clientId);
}

void setup() {
  Serial.begin(9600);

  // Initialize Ethernet with DHCP
  Serial.println(F("Starting Ethernet..."));
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("DHCP failed, using fallback IP"));
    Ethernet.begin(mac, IPAddress(192, 168, 1, 177));
  }

  Serial.print(F("IP: "));
  Serial.println(Ethernet.localIP());

  // Setup WebSocket handlers
  ws.onMessage(onMessage);
  ws.onConnect(onConnect);
  ws.onDisconnect(onDisconnect);

  // Setup routes
  app.get("/", &indexHandler);
  app.get("/ws", &wsHandler);

  server.begin();
  Serial.println(F("Server ready"));
}

void loop() {
  EthernetClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }

  // Forward Serial data to all WebSocket clients
  while (Serial.available()) {
    char c = Serial.read();
    // Broadcast using sendText for text data
    char buf[2] = {c, '\0'};
    ws.broadcastText(buf);
  }

  // Poll WebSocket for incoming messages
  ws.poll();
}
