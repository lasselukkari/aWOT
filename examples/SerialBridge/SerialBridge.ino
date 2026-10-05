/*
  Serial to WebSocket bridge for Arduino Uno + W5100/W5500 Ethernet shield.
  Install the Ethernet 2.x library and adjust the static IP for your network.
  One socket and a 64-byte receive limit leave room for HTTP parsing on Uno.
*/
#include <Ethernet.h>
#include <aWOT.h>

#define WS_RX_BUFFER_SIZE 64
#include <WsChannel.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);
App app;
WsChannel<EthernetClient, 1, 1> sockets;

void indexHandler(Request& req, Response& res) {
  P(page) =
      "<!doctype html><title>Serial bridge</title><input id='text' maxlength='64'>"
      "<button onclick='ws.send(text.value)'>Send</button><pre id='log'></pre>"
      "<script>var ws=new WebSocket('ws://'+location.host+'/ws');"
      "ws.binaryType='arraybuffer';ws.onmessage=e=>log.textContent+="
      "typeof e.data==='string'?e.data:new TextDecoder().decode(e.data);</script>";
  res.set("Content-Type", "text/html");
  res.printP(page);
}

void wsHandler(Request& req, Response& res) {
  if (!sockets.upgrade(req, res)) res.sendStatus(400);
}

void onMessage(WsMessage& msg) {
  while (msg.available()) Serial.write((uint8_t)msg.read());
  Serial.println();
  sockets.send(msg.socketId(), "> OK\n");
}

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  Serial.println(Ethernet.localIP());
  sockets.onMessage(onMessage);
  app.get("/", indexHandler);
  app.get("/ws", wsHandler);
  server.begin();
}

void loop() {
  // accept() removes the socket from the server's pending HTTP clients.
  EthernetClient client = server.accept();
  if (client) {
    char upgrade[16], connection[32], key[25], version[4];
    Request::Header headers[] = {
        {"Upgrade", upgrade},
        {"Connection", connection},
        {"Sec-WebSocket-Key", key},
        {"Sec-WebSocket-Version", version}};
    App::ProcessResult result = app.process(&client, headers);
    if (!result.responseOpen) client.stop();
  }

  sockets.poll();
  while (Serial.available()) {
    uint8_t value = (uint8_t)Serial.read();
    sockets.broadcast(&value, 1, true);
  }
}
