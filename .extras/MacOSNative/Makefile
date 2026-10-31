CXX = clang++
ARDUINO_API = ../ArduinoCore-API/api

# Keep the local shim in dependency files so edits rebuild every user of it.
CXXFLAGS = -std=c++11 -Wall -Wextra -MMD -MP -I../../src -I. -isystem $(ARDUINO_API) -isystem $(ARDUINO_API)/deprecated-avr-comp

ARDUINO_OBJECTS = Print.o Stream.o WString.o IPAddress.o Common.o
APP_OBJECTS = HelloWorld.o MacOSSocket.o
AWOT_OBJECTS = App.o Request.o Response.o Router.o

TARGET = HelloWorld
TEST_TARGETS = SocketTests ShimTests
TEST_OBJECTS = SocketTests.o ShimTests.o

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(APP_OBJECTS) $(ARDUINO_OBJECTS) $(AWOT_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

SocketTests: SocketTests.o MacOSSocket.o $(ARDUINO_OBJECTS) $(AWOT_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

ShimTests: ShimTests.o MacOSSocket.o $(ARDUINO_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

SocketTests.o: tests/sockets.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

ShimTests.o: tests/shims.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

HelloWorld.o: HelloWorld.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

MacOSSocket.o: MacOSSocket.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Print.o: $(ARDUINO_API)/Print.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Stream.o: $(ARDUINO_API)/Stream.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

WString.o: $(ARDUINO_API)/WString.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

IPAddress.o: $(ARDUINO_API)/IPAddress.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Common.o: $(ARDUINO_API)/Common.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

App.o: ../../src/App.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Request.o: ../../src/Request.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Response.o: ../../src/Response.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Router.o: ../../src/Router.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(TEST_TARGETS) *.o *.d

run: $(TARGET)
	./$(TARGET)

test: $(TARGET) $(TEST_TARGETS)
	./ShimTests
	./SocketTests
	@set -e; server_log=$$(mktemp); \
	./$(TARGET) > "$$server_log" 2>&1 & server_pid=$$!; \
	trap 'kill "$$server_pid" 2>/dev/null || true; wait "$$server_pid" 2>/dev/null || true; rm -f "$$server_log"' EXIT; \
	trap 'exit 1' HUP INT TERM; \
	ready=0; \
	for attempt in 1 2 3 4 5; do \
		if ! kill -0 "$$server_pid" 2>/dev/null; then cat "$$server_log"; exit 1; fi; \
		if grep -q '^Server listening on port 8080 ' "$$server_log"; then ready=1; break; fi; \
		sleep 1; \
	done; \
	if [ "$$ready" -eq 0 ]; then cat "$$server_log"; echo "Server did not become ready" >&2; exit 1; fi; \
	response=$$(curl --fail --silent --max-time 10 http://127.0.0.1:8080/); \
	printf '%s' "$$response" | grep -q "Greetings middle earth"; \
	kill -0 "$$server_pid"; \
	echo "Test passed"

-include $(APP_OBJECTS:.o=.d) $(ARDUINO_OBJECTS:.o=.d) $(AWOT_OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)
