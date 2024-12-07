/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

#ifndef EPOXY_DUINO_STDIO_SERIAL_H
#define EPOXY_DUINO_STDIO_SERIAL_H

#include "Print.h"
#include "Stream.h"

/**
 * A version of Serial that reads from STDIN and sends output to STDOUT or
 * STDERR on Linux or MacOS.
 */

// Class to enumerate the two output options: STDOUT or STDERR
enum class StdioOutput { 
  STDOUT, 
  STDERR 
};

class StdioSerial: public Stream {
  public:
    StdioSerial(); // Constructor to set the output when an object is instantiated

    void setOutput(StdioOutput output); // Set the output for write/print methods

    void begin(unsigned long /*baud*/) { bufch = -1; }

    size_t write(uint8_t c) override;

    // Pull in all other overloaded versions of the write() function from the
    // Print parent class. This is required because when we override one version
    // of write() above, C++ performs a static binding to the write() function
    // in the current class and doesn't bother searching the parent classes for
    // any other overloaded function that it could bind to. (30 years of C++ and
    // I still get shot with C++ footguns like this. I have no idea what happens
    // if the Stream class overloaded the write() function.)
    using Print::write;

    operator bool() { return true; }

    int available() override;

    int read() override;

    int peek() override;

    void flush() override; // Force sending of all buffered data

  private:
    int bufch;
    int outputFd; // Store the selected output 
};

extern StdioSerial Serial;

#define SERIAL_PORT_MONITOR Serial

#endif
