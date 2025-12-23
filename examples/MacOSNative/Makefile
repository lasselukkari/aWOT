# macOS Native aWOT Example
CXX = clang++

# Include paths:
# Use -isystem for ArduinoCore to treat as system headers
# Use -iquote for local Arduino.h to be found with quotes (via aWOT)
CXXFLAGS = -std=c++11 -Wall -Wextra -I../../src -isystem . -isystem ArduinoCore-API/api -isystem ArduinoCore-API/api/deprecated-avr-comp

# Arduino core source files (from ArduinoCore-API)
ARDUINO_SOURCES = ArduinoCore-API/api/Print.cpp \
                  ArduinoCore-API/api/Stream.cpp \
                  ArduinoCore-API/api/WString.cpp \
                  ArduinoCore-API/api/IPAddress.cpp \
                  ArduinoCore-API/api/Common.cpp

# Application source files
APP_SOURCES = HelloWorld.cpp MacOSSocket.cpp

# aWOT source files
AWOT_SOURCES = ../../src/App.cpp ../../src/Request.cpp ../../src/Response.cpp ../../src/Router.cpp

# Object files
ARDUINO_OBJECTS = Print.o Stream.o WString.o IPAddress.o Common.o
APP_OBJECTS = HelloWorld.o MacOSSocket.o
AWOT_OBJECTS = App.o Request.o Response.o Router.o

# Target
TARGET = HelloWorld

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(APP_OBJECTS) $(ARDUINO_OBJECTS) $(AWOT_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Application objects
HelloWorld.o: HelloWorld.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

MacOSSocket.o: MacOSSocket.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Arduino Core objects
Print.o: ArduinoCore-API/api/Print.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Stream.o: ArduinoCore-API/api/Stream.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

WString.o: ArduinoCore-API/api/WString.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

IPAddress.o: ArduinoCore-API/api/IPAddress.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Common.o: ArduinoCore-API/api/Common.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# aWOT objects
App.o: ../../src/App.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Request.o: ../../src/Request.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Response.o: ../../src/Response.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Router.o: ../../src/Router.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	@echo "Starting server in background..."
	@./$(TARGET) & echo $$! > .server.pid
	@sleep 1
	@echo "Testing GET /..."
	@curl -s http://localhost:8080/ | grep -q "Greetings middle earth" && echo "✓ Test passed!" || echo "✗ Test failed!"
	@echo "Stopping server..."
	@kill $$(cat .server.pid) 2>/dev/null || true
	@rm -f .server.pid
