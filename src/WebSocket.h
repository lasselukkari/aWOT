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

#ifndef AWOT_WEBSOCKET_H_
#define AWOT_WEBSOCKET_H_

#include "aWOT_common.h"

namespace awot {

// Forward declarations
class WebSocket;

// Incoming WebSocket message - extends Stream for read/write
class WebSocketMessage : public Stream {
  friend class WebSocket;

 public:
  // Message info
  int clientId() const { return m_clientId; }
  bool isBinary() const { return m_binary; }
  size_t length() const { return m_length; }

  // Stream read interface (from incoming message)
  int available() override;
  int read() override;
  int peek() override;

  // Stream write interface (response to client)
  size_t write(uint8_t data) override;
  size_t write(const uint8_t* buf, size_t size) override;
  void flush() override;

 private:
  WebSocketMessage(WebSocket* ws, Client* client, int clientId, bool binary,
                   size_t length, const uint8_t* mask);

  WebSocket* m_ws;
  Client* m_client;
  int m_clientId;
  bool m_binary;
  size_t m_length;     // Total payload length
  size_t m_remaining;  // Bytes left to read
  uint8_t m_mask[4];   // XOR mask for unmasking
  size_t m_maskPos;    // Current mask position
  int m_peeked;        // Peeked byte (-1 if none)
};

// Options for poll() - like App::ProcessOptions
struct PollOptions {
  uint8_t* buffer = nullptr;
  int bufferLength = 0;
};

// Main WebSocket server
class WebSocket {
  friend class App;
  friend class WebSocketMessage;

 public:
  // Handler types
  typedef void (*MessageHandler)(WebSocketMessage& msg);
  typedef void (*ConnectHandler)(int clientId);

  WebSocket();

  // Event handlers
  void onMessage(MessageHandler handler);
  void onConnect(ConnectHandler handler);
  void onDisconnect(ConnectHandler handler);

  // Attempt WebSocket upgrade from HTTP request
  bool upgrade(Client* client, const char* key);

  // Poll for messages (call in loop())
  void poll();
  void poll(const PollOptions& options);

  // Send to specific client
  void send(int clientId, const uint8_t* data, size_t length);
  void sendText(int clientId, const char* data);
  void sendBinary(int clientId, const uint8_t* data, size_t length);

  // Broadcast to all connected clients
  void broadcast(const uint8_t* data, size_t length);
  void broadcastText(const char* data);
  void broadcastBinary(const uint8_t* data, size_t length);

  // Room management
  void join(int clientId, const char* room);
  void leave(int clientId, const char* room);
  void to(const char* room, const uint8_t* data, size_t length);
  void toText(const char* room, const char* data);

  // Client management
  int clientCount();
  bool connected(int clientId);

 private:
  struct ClientNode {
    Client* client;
    bool connected;
    uint8_t rooms;           // Bitmask for room membership
    bool inFragment;         // Currently receiving fragmented message
    uint8_t fragmentOpcode;  // Original opcode for fragments
  };

  ClientNode m_clients[WEBSOCKET_MAX_CLIENTS];
  const char* m_roomNames[WEBSOCKET_MAX_ROOMS];
  int m_roomCount;
  MessageHandler m_messageHandler;
  ConnectHandler m_connectHandler;
  ConnectHandler m_disconnectHandler;

  int m_findRoom(const char* room);
  int m_addRoom(const char* room);
  bool m_sendHandshake(Client* client, const char* key);
  int m_findSlot();
  void m_sendFrame(Client* client, uint8_t opcode, const uint8_t* data,
                   size_t len);
  bool m_readFrame(int clientId, uint8_t* buffer, int bufferLength);
  void m_handlePing(int clientId, const uint8_t* data, size_t len);
  void m_closeClient(int clientId);
  void m_sha1(const uint8_t* data, size_t len, uint8_t* hash);
  void m_base64Encode(const uint8_t* input, size_t inputLen, char* output);
};

}  // namespace awot

#endif
