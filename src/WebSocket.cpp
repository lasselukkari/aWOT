/*
  aWOT, Express.js inspired microcontroller web framework for the Web of Things

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "WebSocket.h"

#include <string.h>

#include "aWOT_common.h"

namespace awot {

// WebSocket GUID for handshake (RFC 6455)
static const char WS_GUID[] PROGMEM = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// Base64 encoding table
static const char BASE64_CHARS[] PROGMEM =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Handshake response strings in PROGMEM
static const char HTTP_101[] PROGMEM = "HTTP/1.1 101 Switching Protocols\r\n";
static const char UPGRADE_WS[] PROGMEM = "Upgrade: websocket\r\n";
static const char CONNECTION_UPGRADE[] PROGMEM = "Connection: Upgrade\r\n";
static const char SEC_ACCEPT[] PROGMEM = "Sec-WebSocket-Accept: ";
static const char CRLF_STR[] PROGMEM = "\r\n";

// -----------------------------------------------------------------------------
// WebSocketMessage implementation
// -----------------------------------------------------------------------------

WebSocketMessage::WebSocketMessage(WebSocket* ws, Client* client, int clientId,
                                   bool binary, size_t length,
                                   const uint8_t* mask)
    : m_ws(ws),
      m_client(client),
      m_clientId(clientId),
      m_binary(binary),
      m_length(length),
      m_remaining(length),
      m_maskPos(0),
      m_peeked(-1) {
  memcpy(m_mask, mask, 4);
}

int WebSocketMessage::available() {
  if (m_peeked >= 0) return m_remaining + 1;
  return m_remaining;
}

int WebSocketMessage::read() {
  // Return peeked byte if available
  if (m_peeked >= 0) {
    int byte = m_peeked;
    m_peeked = -1;
    return byte;
  }

  if (m_remaining == 0) return -1;

  int byte = m_client->read();
  if (byte < 0) return -1;

  m_remaining--;
  // Unmask on-the-fly
  return byte ^ m_mask[m_maskPos++ % 4];
}

int WebSocketMessage::peek() {
  if (m_peeked >= 0) return m_peeked;

  int byte = read();
  if (byte >= 0) {
    m_peeked = byte;
  }
  return byte;
}

size_t WebSocketMessage::write(uint8_t data) {
  // Write single byte as binary frame
  m_ws->send(m_clientId, &data, 1);
  return 1;
}

size_t WebSocketMessage::write(const uint8_t* buf, size_t size) {
  m_ws->send(m_clientId, buf, size);
  return size;
}

void WebSocketMessage::flush() {
  // Nothing to flush - sends are immediate
}

// -----------------------------------------------------------------------------
// WebSocket implementation
// -----------------------------------------------------------------------------

WebSocket::WebSocket()
    : m_roomCount(0),
      m_messageHandler(nullptr),
      m_connectHandler(nullptr),
      m_disconnectHandler(nullptr) {
  for (int i = 0; i < WEBSOCKET_MAX_CLIENTS; i++) {
    m_clients[i].client = nullptr;
    m_clients[i].connected = false;
    m_clients[i].rooms = 0;
    m_clients[i].inFragment = false;
    m_clients[i].fragmentOpcode = 0;
  }
  for (int i = 0; i < WEBSOCKET_MAX_ROOMS; i++) {
    m_roomNames[i] = nullptr;
  }
}

void WebSocket::onMessage(MessageHandler handler) {
  m_messageHandler = handler;
}

void WebSocket::onConnect(ConnectHandler handler) {
  m_connectHandler = handler;
}

void WebSocket::onDisconnect(ConnectHandler handler) {
  m_disconnectHandler = handler;
}

bool WebSocket::upgrade(Client* client, const char* key) {
  int slot = m_findSlot();
  if (slot < 0) {
    return false;
  }

  if (!m_sendHandshake(client, key)) {
    return false;
  }

  m_clients[slot].client = client;
  m_clients[slot].connected = true;
  m_clients[slot].rooms = 0;
  m_clients[slot].inFragment = false;

  if (m_connectHandler) {
    m_connectHandler(slot);
  }

  return true;
}

void WebSocket::poll() {
  // Stack-allocated default buffer
  uint8_t defaultBuffer[WEBSOCKET_BUFFER_SIZE];
  PollOptions options;
  options.buffer = defaultBuffer;
  options.bufferLength = WEBSOCKET_BUFFER_SIZE;
  poll(options);
}

void WebSocket::poll(const PollOptions& options) {
  for (int i = 0; i < WEBSOCKET_MAX_CLIENTS; i++) {
    if (!m_clients[i].connected || !m_clients[i].client) {
      continue;
    }

    Client* client = m_clients[i].client;

    if (!client->connected()) {
      m_closeClient(i);
      continue;
    }

    if (client->available() > 0) {
      m_readFrame(i, options.buffer, options.bufferLength);
    }
  }
}

void WebSocket::send(int clientId, const uint8_t* data, size_t length) {
  sendBinary(clientId, data, length);
}

void WebSocket::sendText(int clientId, const char* data) {
  if (clientId < 0 || clientId >= WEBSOCKET_MAX_CLIENTS) return;
  if (!m_clients[clientId].connected || !m_clients[clientId].client) return;
  m_sendFrame(m_clients[clientId].client, WS_OPCODE_TEXT, (const uint8_t*)data,
              strlen(data));
}

void WebSocket::sendBinary(int clientId, const uint8_t* data, size_t length) {
  if (clientId < 0 || clientId >= WEBSOCKET_MAX_CLIENTS) return;
  if (!m_clients[clientId].connected || !m_clients[clientId].client) return;
  m_sendFrame(m_clients[clientId].client, WS_OPCODE_BINARY, data, length);
}

void WebSocket::broadcast(const uint8_t* data, size_t length) {
  broadcastBinary(data, length);
}

void WebSocket::broadcastText(const char* data) {
  size_t len = strlen(data);
  for (int i = 0; i < WEBSOCKET_MAX_CLIENTS; i++) {
    if (m_clients[i].connected && m_clients[i].client) {
      m_sendFrame(m_clients[i].client, WS_OPCODE_TEXT, (const uint8_t*)data,
                  len);
    }
  }
}

void WebSocket::broadcastBinary(const uint8_t* data, size_t length) {
  for (int i = 0; i < WEBSOCKET_MAX_CLIENTS; i++) {
    if (m_clients[i].connected && m_clients[i].client) {
      m_sendFrame(m_clients[i].client, WS_OPCODE_BINARY, data, length);
    }
  }
}

void WebSocket::join(int clientId, const char* room) {
  if (clientId < 0 || clientId >= WEBSOCKET_MAX_CLIENTS) return;
  int roomIdx = m_findRoom(room);
  if (roomIdx < 0) {
    roomIdx = m_addRoom(room);
  }
  if (roomIdx >= 0) {
    m_clients[clientId].rooms |= (1 << roomIdx);
  }
}

void WebSocket::leave(int clientId, const char* room) {
  if (clientId < 0 || clientId >= WEBSOCKET_MAX_CLIENTS) return;
  int roomIdx = m_findRoom(room);
  if (roomIdx >= 0) {
    m_clients[clientId].rooms &= ~(1 << roomIdx);
  }
}

void WebSocket::to(const char* room, const uint8_t* data, size_t length) {
  int roomIdx = m_findRoom(room);
  if (roomIdx < 0) return;
  uint8_t roomMask = (1 << roomIdx);
  for (int i = 0; i < WEBSOCKET_MAX_CLIENTS; i++) {
    if (m_clients[i].connected && m_clients[i].client &&
        (m_clients[i].rooms & roomMask)) {
      m_sendFrame(m_clients[i].client, WS_OPCODE_BINARY, data, length);
    }
  }
}

void WebSocket::toText(const char* room, const char* data) {
  int roomIdx = m_findRoom(room);
  if (roomIdx < 0) return;
  uint8_t roomMask = (1 << roomIdx);
  size_t len = strlen(data);
  for (int i = 0; i < WEBSOCKET_MAX_CLIENTS; i++) {
    if (m_clients[i].connected && m_clients[i].client &&
        (m_clients[i].rooms & roomMask)) {
      m_sendFrame(m_clients[i].client, WS_OPCODE_TEXT, (const uint8_t*)data,
                  len);
    }
  }
}

int WebSocket::clientCount() {
  int count = 0;
  for (int i = 0; i < WEBSOCKET_MAX_CLIENTS; i++) {
    if (m_clients[i].connected) count++;
  }
  return count;
}

bool WebSocket::connected(int clientId) {
  if (clientId < 0 || clientId >= WEBSOCKET_MAX_CLIENTS) return false;
  return m_clients[clientId].connected;
}

int WebSocket::m_findRoom(const char* room) {
  for (int i = 0; i < m_roomCount; i++) {
    if (m_roomNames[i] && strcmp(m_roomNames[i], room) == 0) {
      return i;
    }
  }
  return -1;
}

int WebSocket::m_addRoom(const char* room) {
  if (m_roomCount >= WEBSOCKET_MAX_ROOMS) return -1;
  m_roomNames[m_roomCount] = room;
  return m_roomCount++;
}

int WebSocket::m_findSlot() {
  for (int i = 0; i < WEBSOCKET_MAX_CLIENTS; i++) {
    if (!m_clients[i].connected) {
      return i;
    }
  }
  return -1;
}

void WebSocket::m_closeClient(int clientId) {
  if (clientId < 0 || clientId >= WEBSOCKET_MAX_CLIENTS) return;

  if (m_disconnectHandler && m_clients[clientId].connected) {
    m_disconnectHandler(clientId);
  }

  m_clients[clientId].connected = false;
  m_clients[clientId].client = nullptr;
  m_clients[clientId].rooms = 0;
  m_clients[clientId].inFragment = false;
}

// Minimal SHA-1 implementation for WebSocket handshake
void WebSocket::m_sha1(const uint8_t* data, size_t len, uint8_t* hash) {
  uint32_t h0 = 0x67452301;
  uint32_t h1 = 0xEFCDAB89;
  uint32_t h2 = 0x98BADCFE;
  uint32_t h3 = 0x10325476;
  uint32_t h4 = 0xC3D2E1F0;

  // Use stack buffer for small messages (key + GUID = ~60 bytes max)
  // Padded length for 60 bytes input = 64 bytes
  uint8_t padded[128];
  size_t paddedLen = ((len + 9 + 63) / 64) * 64;

  if (paddedLen > sizeof(padded)) {
    // Message too large for stack buffer - shouldn't happen for handshake
    return;
  }

  memset(padded, 0, paddedLen);
  memcpy(padded, data, len);
  padded[len] = 0x80;

  // Append original length in bits (big-endian)
  uint64_t bitLen = len * 8;
  for (int i = 0; i < 8; i++) {
    padded[paddedLen - 1 - i] = (bitLen >> (i * 8)) & 0xFF;
  }

  // Process each 64-byte block
  for (size_t block = 0; block < paddedLen; block += 64) {
    uint32_t w[80];

    for (int i = 0; i < 16; i++) {
      w[i] = ((uint32_t)padded[block + i * 4] << 24) |
             ((uint32_t)padded[block + i * 4 + 1] << 16) |
             ((uint32_t)padded[block + i * 4 + 2] << 8) |
             ((uint32_t)padded[block + i * 4 + 3]);
    }
    for (int i = 16; i < 80; i++) {
      uint32_t temp = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
      w[i] = (temp << 1) | (temp >> 31);
    }

    uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;

    for (int i = 0; i < 80; i++) {
      uint32_t f, k;
      if (i < 20) {
        f = (b & c) | ((~b) & d);
        k = 0x5A827999;
      } else if (i < 40) {
        f = b ^ c ^ d;
        k = 0x6ED9EBA1;
      } else if (i < 60) {
        f = (b & c) | (b & d) | (c & d);
        k = 0x8F1BBCDC;
      } else {
        f = b ^ c ^ d;
        k = 0xCA62C1D6;
      }

      uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
      e = d;
      d = c;
      c = (b << 30) | (b >> 2);
      b = a;
      a = temp;
    }

    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;
    h4 += e;
  }

  // Produce final hash (big-endian)
  hash[0] = (h0 >> 24) & 0xFF;
  hash[1] = (h0 >> 16) & 0xFF;
  hash[2] = (h0 >> 8) & 0xFF;
  hash[3] = h0 & 0xFF;
  hash[4] = (h1 >> 24) & 0xFF;
  hash[5] = (h1 >> 16) & 0xFF;
  hash[6] = (h1 >> 8) & 0xFF;
  hash[7] = h1 & 0xFF;
  hash[8] = (h2 >> 24) & 0xFF;
  hash[9] = (h2 >> 16) & 0xFF;
  hash[10] = (h2 >> 8) & 0xFF;
  hash[11] = h2 & 0xFF;
  hash[12] = (h3 >> 24) & 0xFF;
  hash[13] = (h3 >> 16) & 0xFF;
  hash[14] = (h3 >> 8) & 0xFF;
  hash[15] = h3 & 0xFF;
  hash[16] = (h4 >> 24) & 0xFF;
  hash[17] = (h4 >> 16) & 0xFF;
  hash[18] = (h4 >> 8) & 0xFF;
  hash[19] = h4 & 0xFF;
}

void WebSocket::m_base64Encode(const uint8_t* input, size_t inputLen,
                               char* output) {
  size_t i = 0;
  size_t j = 0;

  while (i < inputLen) {
    uint32_t octet_a = i < inputLen ? input[i++] : 0;
    uint32_t octet_b = i < inputLen ? input[i++] : 0;
    uint32_t octet_c = i < inputLen ? input[i++] : 0;

    uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

    output[j++] = pgm_read_byte(BASE64_CHARS + ((triple >> 18) & 0x3F));
    output[j++] = pgm_read_byte(BASE64_CHARS + ((triple >> 12) & 0x3F));
    output[j++] = pgm_read_byte(BASE64_CHARS + ((triple >> 6) & 0x3F));
    output[j++] = pgm_read_byte(BASE64_CHARS + (triple & 0x3F));
  }

  // Add padding
  size_t mod = inputLen % 3;
  if (mod == 1) {
    output[j - 2] = '=';
    output[j - 1] = '=';
  } else if (mod == 2) {
    output[j - 1] = '=';
  }

  output[j] = '\0';
}

bool WebSocket::m_sendHandshake(Client* client, const char* key) {
  // Concatenate key + GUID
  char combined[64];
  size_t keyLen = strlen(key);

  // Copy GUID from PROGMEM
  char guid[40];
  for (size_t i = 0; i < sizeof(guid) - 1; i++) {
    char c = pgm_read_byte(WS_GUID + i);
    if (c == 0) {
      guid[i] = 0;
      break;
    }
    guid[i] = c;
  }
  size_t guidLen = strlen(guid);

  if (keyLen + guidLen >= sizeof(combined)) {
    return false;
  }

  memcpy(combined, key, keyLen);
  memcpy(combined + keyLen, guid, guidLen + 1);

  // Compute SHA-1 hash
  uint8_t hash[20];
  m_sha1((const uint8_t*)combined, keyLen + guidLen, hash);

  // Base64 encode
  char acceptKey[32];
  m_base64Encode(hash, 20, acceptKey);

  // Send HTTP response using PROGMEM strings
  client->print((__FlashStringHelper*)HTTP_101);
  client->print((__FlashStringHelper*)UPGRADE_WS);
  client->print((__FlashStringHelper*)CONNECTION_UPGRADE);
  client->print((__FlashStringHelper*)SEC_ACCEPT);
  client->print(acceptKey);
  client->print((__FlashStringHelper*)CRLF);
  client->print((__FlashStringHelper*)CRLF);

  return true;
}

void WebSocket::m_sendFrame(Client* client, uint8_t opcode, const uint8_t* data,
                            size_t len) {
  // FIN bit set, opcode
  client->write(0x80 | opcode);

  // Payload length (server-to-client frames are unmasked)
  if (len < 126) {
    client->write((uint8_t)len);
  } else if (len <= 0xFFFF) {
    client->write((uint8_t)126);
    client->write((uint8_t)(len >> 8));
    client->write((uint8_t)(len & 0xFF));
  } else {
    client->write((uint8_t)127);
    for (int i = 7; i >= 0; i--) {
      client->write((uint8_t)((len >> (i * 8)) & 0xFF));
    }
  }

  // Payload data
  client->write(data, len);
}

bool WebSocket::m_readFrame(int clientId, uint8_t* buffer, int bufferLength) {
  Client* client = m_clients[clientId].client;

  if (client->available() < 2) {
    return false;
  }

  int firstByte = client->read();
  int secondByte = client->read();

  if (firstByte < 0 || secondByte < 0) {
    return false;
  }

  bool fin = (firstByte & 0x80) != 0;
  uint8_t opcode = firstByte & 0x0F;
  bool masked = (secondByte & 0x80) != 0;
  size_t payloadLen = secondByte & 0x7F;

  // Handle continuation frames
  if (opcode == 0x00) {
    // Continuation - use stored opcode
    opcode = m_clients[clientId].fragmentOpcode;
  } else if (!fin) {
    // First fragment - store opcode
    m_clients[clientId].inFragment = true;
    m_clients[clientId].fragmentOpcode = opcode;
  }

  if (fin && m_clients[clientId].inFragment) {
    m_clients[clientId].inFragment = false;
  }

  // Extended payload length
  if (payloadLen == 126) {
    if (client->available() < 2) return false;
    int high = client->read();
    int low = client->read();
    if (high < 0 || low < 0) return false;
    payloadLen = ((size_t)high << 8) | (size_t)low;
  } else if (payloadLen == 127) {
    if (client->available() < 8) return false;
    for (int i = 0; i < 4; i++) client->read();  // Skip high bytes
    payloadLen = 0;
    for (int i = 0; i < 4; i++) {
      int byte = client->read();
      if (byte < 0) return false;
      payloadLen = (payloadLen << 8) | (size_t)byte;
    }
  }

  // Read masking key
  uint8_t mask[4] = {0, 0, 0, 0};
  if (masked) {
    if (client->available() < 4) return false;
    for (int i = 0; i < 4; i++) {
      int byte = client->read();
      if (byte < 0) return false;
      mask[i] = (uint8_t)byte;
    }
  }

  // Handle control frames (PING, PONG, CLOSE) - need to buffer these
  if (opcode == WS_OPCODE_CLOSE) {
    m_closeClient(clientId);
    return true;
  }

  if (opcode == WS_OPCODE_PING) {
    // Read ping payload to buffer
    size_t readLen =
        payloadLen < (size_t)bufferLength ? payloadLen : bufferLength;
    for (size_t i = 0; i < readLen; i++) {
      int byte = client->read();
      if (byte < 0) return false;
      buffer[i] = masked ? (uint8_t)byte ^ mask[i % 4] : (uint8_t)byte;
    }
    // Skip remaining
    for (size_t i = readLen; i < payloadLen; i++) client->read();
    m_handlePing(clientId, buffer, readLen);
    return true;
  }

  if (opcode == WS_OPCODE_PONG) {
    // Skip pong payload
    for (size_t i = 0; i < payloadLen; i++) client->read();
    return true;
  }

  // Data frame - create WebSocketMessage for streaming read
  if (opcode == WS_OPCODE_TEXT || opcode == WS_OPCODE_BINARY) {
    if (m_messageHandler) {
      bool isBinary = (opcode == WS_OPCODE_BINARY);
      WebSocketMessage msg(this, client, clientId, isBinary, payloadLen, mask);
      m_messageHandler(msg);

      // Consume any unread bytes
      while (msg.available() > 0) {
        msg.read();
      }
    } else {
      // No handler - skip payload
      for (size_t i = 0; i < payloadLen; i++) client->read();
    }
    return true;
  }

  return false;
}

void WebSocket::m_handlePing(int clientId, const uint8_t* data, size_t len) {
  if (!m_clients[clientId].connected || !m_clients[clientId].client) {
    return;
  }
  m_sendFrame(m_clients[clientId].client, WS_OPCODE_PONG, data, len);
}

}  // namespace awot
