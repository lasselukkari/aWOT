/*
  ChatRoom.ino - WebSocket chat room example for aWOT

  This example demonstrates using WebSocket for real-time
  bidirectional communication to create a simple chat room.

  The example works with WiFi-enabled boards (ESP8266, ESP32).
*/

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <aWOT.h>
#include <WsChannel.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

WiFiServer server(80);
App app;
WsChannel<WiFiClient, 4, 1> sockets;
int chatRoom;

// HTML page stored in PROGMEM
P(indexHtml) =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "  <meta charset=\"UTF-8\">\n"
    "  <meta name=\"viewport\" content=\"width=device-width, "
    "initial-scale=1\">\n"
    "  <title>aWOT Chat Room</title>\n"
    "  <style>\n"
    "    * { box-sizing: border-box; margin: 0; padding: 0; }\n"
    "    body { font-family: system-ui, sans-serif; background: #1a1a2e; "
    "color: #eee; height: 100vh; display: flex; flex-direction: column; }\n"
    "    header { background: #16213e; padding: 1rem; text-align: center; }\n"
    "    header h1 { font-size: 1.5rem; color: #00fff5; }\n"
    "    #status { font-size: 0.8rem; margin-top: 0.5rem; }\n"
    "    .connected { color: #4ade80; }\n"
    "    .disconnected { color: #f87171; }\n"
    "    #messages { flex: 1; overflow-y: auto; padding: 1rem; display: flex; "
    "flex-direction: column; gap: 0.5rem; }\n"
    "    .message { background: #16213e; padding: 0.75rem 1rem; border-radius: "
    "8px; max-width: 80%; }\n"
    "    .message.self { background: #0f3460; align-self: flex-end; }\n"
    "    .message .user { color: #00fff5; font-weight: bold; font-size: "
    "0.85rem; }\n"
    "    .message .text { margin-top: 0.25rem; word-wrap: break-word; }\n"
    "    .message .time { color: #888; font-size: 0.7rem; margin-top: 0.25rem; "
    "}\n"
    "    #input-area { background: #16213e; padding: 1rem; display: flex; gap: "
    "0.5rem; }\n"
    "    input, button { font-size: 1rem; padding: 0.75rem 1rem; border: none; "
    "border-radius: 8px; }\n"
    "    input { flex: 1; background: #1a1a2e; color: #eee; }\n"
    "    button { background: #00fff5; color: #1a1a2e; font-weight: bold; "
    "cursor: pointer; }\n"
    "    button:hover { background: #00d4d4; }\n"
    "    #username-area { padding: 1rem; background: #0f3460; display: flex; "
    "gap: 0.5rem; }\n"
    "  </style>\n"
    "</head>\n"
    "<body>\n"
    "  <header>\n"
    "    <h1>aWOT Chat Room</h1>\n"
    "    <div id=\"status\" class=\"disconnected\">Connecting...</div>\n"
    "  </header>\n"
    "  <div id=\"username-area\">\n"
    "    <input type=\"text\" id=\"username\" placeholder=\"Enter your name\" "
    "value=\"Guest\">\n"
    "    <button onclick=\"setUsername()\">Set Name</button>\n"
    "  </div>\n"
    "  <div id=\"messages\"></div>\n"
    "  <div id=\"input-area\">\n"
    "    <input type=\"text\" id=\"msg\" placeholder=\"Type a message...\" "
    "onkeypress=\"if(event.key==='Enter')sendMsg()\">\n"
    "    <button onclick=\"sendMsg()\">Send</button>\n"
    "  </div>\n"
    "  <script>\n"
    "    let ws, username = 'Guest';\n"
    "    const status = document.getElementById('status');\n"
    "    const messages = document.getElementById('messages');\n"
    "    const msgInput = document.getElementById('msg');\n"
    "\n"
    "    function connect() {\n"
    "      const proto = location.protocol === 'https:' ? 'wss:' : 'ws:';\n"
    "      ws = new WebSocket(proto + '//' + location.host + '/ws');\n"
    "      ws.onopen = () => {\n"
    "        status.textContent = 'Connected';\n"
    "        status.className = 'connected';\n"
    "      };\n"
    "      ws.onclose = () => {\n"
    "        status.textContent = 'Disconnected - Reconnecting...';\n"
    "        status.className = 'disconnected';\n"
    "        setTimeout(connect, 2000);\n"
    "      };\n"
    "      ws.onerror = () => ws.close();\n"
    "      ws.onmessage = (e) => {\n"
    "        try {\n"
    "          const data = JSON.parse(e.data);\n"
    "          addMessage(data.user, data.text, data.user === username);\n"
    "        } catch(err) {\n"
    "          addMessage('System', e.data, false);\n"
    "        }\n"
    "      };\n"
    "    }\n"
    "\n"
    "    function addMessage(user, text, self) {\n"
    "      const div = document.createElement('div');\n"
    "      div.className = 'message' + (self ? ' self' : '');\n"
    "      const time = new Date().toLocaleTimeString();\n"
    "      div.innerHTML = '<div class=\"user\">' + user + '</div><div "
    "class=\"text\">' + text + '</div><div class=\"time\">' + time + "
    "'</div>';\n"
    "      messages.appendChild(div);\n"
    "      messages.scrollTop = messages.scrollHeight;\n"
    "    }\n"
    "\n"
    "    function sendMsg() {\n"
    "      const text = msgInput.value.trim();\n"
    "      if (!text || ws.readyState !== 1) return;\n"
    "      ws.send(JSON.stringify({ user: username, text: text }));\n"
    "      msgInput.value = '';\n"
    "    }\n"
    "\n"
    "    function setUsername() {\n"
    "      const input = document.getElementById('username').value.trim();\n"
    "      if (input) username = input;\n"
    "    }\n"
    "\n"
    "    connect();\n"
    "  </script>\n"
    "</body>\n"
    "</html>\n";

void indexHandler(Request& req, Response& res) {
  res.set("Content-Type", "text/html");
  res.printP(indexHtml);
}

void wsHandler(Request& req, Response& res) {
  if (!sockets.upgrade(req, res)) res.sendStatus(400);
}

void onMessage(WsMessage& msg) {
  sockets.to(chatRoom, msg.data(), msg.length(), msg.isBinary());
}

void onConnect(int clientId) {
  Serial.print("Client connected: ");
  Serial.println(clientId);

  sockets.join(clientId, chatRoom);

  const char* welcome =
      "{\"user\":\"System\",\"text\":\"Welcome to the chat room!\"}";
  sockets.send(clientId, welcome);
}

void onDisconnect(int clientId) {
  Serial.print("Client disconnected: ");
  Serial.println(clientId);
}

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  chatRoom = sockets.defineRoom("chat");

  sockets.onMessage(onMessage);
  sockets.onConnect(onConnect);
  sockets.onDisconnect(onDisconnect);

  app.get("/", &indexHandler);
  app.get("/ws", &wsHandler);

  server.begin();
  Serial.println("Server started");
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    char upgrade[16], connection[64], key[25], version[4];
    Request::HeaderNode headers[] = {
        {"Upgrade", upgrade, sizeof(upgrade), nullptr},
        {"Connection", connection, sizeof(connection), nullptr},
        {"Sec-WebSocket-Key", key, sizeof(key), nullptr},
        {"Sec-WebSocket-Version", version, sizeof(version), nullptr}};
    App::ProcessOptions options;
    options.headers = headers;
    options.headerCount = 4;
    App::ProcessResult result = app.process(&client, options);
    if (!result.responseOpen) client.stop();
  }

  sockets.poll();
}
