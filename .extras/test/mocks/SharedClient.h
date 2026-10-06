#pragma once

#include <Client.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

// Copies share the same transport, as EthernetClient/WiFiClient copies do.
// Intentionally implements only the standard Arduino Client contract.
class SharedClient : public Client {
 public:
  struct State {
    std::vector<uint8_t> input;
    std::vector<uint8_t> output;
    size_t position = 0;
    bool live = true;
    size_t maxWrite = (size_t)-1;
    size_t failAfter = (size_t)-1;
  };

  std::shared_ptr<State> state;

  SharedClient() : state(new State) {}
  int connect(IPAddress, uint16_t) override { return 1; }
  int connect(const char*, uint16_t) override { return 1; }
  size_t write(uint8_t value) override { return write(&value, 1); }
  size_t write(const uint8_t* data, size_t length) override {
    if (!state->live || state->output.size() >= state->failAfter) return 0;
    length = std::min(length, state->maxWrite);
    length = std::min(length, state->failAfter - state->output.size());
    state->output.insert(state->output.end(), data, data + length);
    return length;
  }
  int available() override {
    return (int)(state->input.size() - state->position);
  }
  int read() override {
    return available() ? state->input[state->position++] : -1;
  }
  int read(uint8_t* data, size_t length) override {
    size_t i = 0;
    while (i < length && available()) data[i++] = (uint8_t)read();
    return (int)i;
  }
  int peek() override {
    return available() ? state->input[state->position] : -1;
  }
  void flush() override {}
  void stop() override { state->live = false; }
  uint8_t connected() override { return state->live; }
  operator bool() override { return state->live; }

  void feed(const std::vector<uint8_t>& data) {
    state->input = data;
    state->position = 0;
  }
  void feed(const std::string& data) {
    feed(std::vector<uint8_t>(data.begin(), data.end()));
  }
  void clearOutput() { state->output.clear(); }
  std::string outputString() const {
    return std::string(state->output.begin(), state->output.end());
  }
};
