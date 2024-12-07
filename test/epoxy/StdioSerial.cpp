/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

#include <stdio.h>
#include <unistd.h>
#include "StdioSerial.h"

// Set default output when compiling
#ifndef STDERR_OUTPUT
#define SERIAL_OUTPUT StdioOutput::STDOUT
#else 
#define SERIAL_OUTPUT StdioOutput::STDERR
#endif

 // Constructor to set the output when an object is instantiated
StdioSerial::StdioSerial() { setOutput(SERIAL_OUTPUT); }

// Select the output at runtime
void StdioSerial::setOutput(StdioOutput output) {
  if (output == StdioOutput::STDOUT) {
    outputFd = STDOUT_FILENO;
  } else if (output == StdioOutput::STDERR) {
    outputFd = STDERR_FILENO;
  }
}

// Force sending of all buffered data
void StdioSerial::flush() {
  fsync(outputFd);
}

// Write to selected output
size_t StdioSerial::write(uint8_t c) {
  ssize_t status = ::write(outputFd, &c, 1);
  return (status <= 0) ? 0 : 1;
}

int StdioSerial::read() {
  int ch = peek();
  bufch = -1;
  return ch;
}

int StdioSerial::peek() {
  if (bufch == -1) {
    // 'c' must be unsigned to avoid ambiguity with -1 in-band error condition
    unsigned char c;
    ssize_t status = ::read(STDIN_FILENO, &c, 1);
    bufch = (status <= 0) ? -1 : c;
  }
  return bufch;
}

int StdioSerial::available() {
  int ch = peek();
  return (int) (ch != -1);
}

StdioSerial Serial;
