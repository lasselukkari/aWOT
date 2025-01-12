/*
 * Copyright (c) 2019 Brian T. Park
 *
 * Parts derived from the Arduino SDK
 * Copyright (c) 2005-2013 Arduino Team
 *
 * Parts inspired by [Entering raw
 * mode](https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html).
 *
 * Parts inspired by [ESP8266 Host
 * Emulation](https://github.com/esp8266/Arduino/tree/master/tests/host).
 *
 * The 'Serial' object sends output to STDOUT, and receives input from STDIN in
 * 'raw' mode. The main() loop checks the STDIN and if it finds a character,
 * inserts it into the Serial buffer.
 */

#ifdef EPOXY_DUINO

#include "Arduino.h"
#include <inttypes.h>
#include <signal.h> // SIGINT
#include <stdlib.h> // exit()
#include <stdio.h> // perror()
#include <unistd.h> // isatty(), STDIN_FILENO, STDOUT_FILENO
#include <fcntl.h>
#include <termios.h>

// -----------------------------------------------------------------------
// Unix compatibility. Put STDIN into raw mode and hook it into the 'Serial'
// object. Trap Ctrl-C and perform appropriate clean up.
// -----------------------------------------------------------------------

static struct termios orig_termios;
static int orig_stdin_flags;
static bool inRawMode = false;
static bool inNonBlockingMode = false;

static void die(const char* s) {
  perror(s);
  exit(1);
}

static void disableRawMode() {
  if (inRawMode) {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
      inRawMode = false; // prevent exit(1) from being called twice
      perror("disableRawMode(): tcsetattr() failure");
    }
  }

  if (inNonBlockingMode) {
    if (fcntl(STDIN_FILENO, F_SETFL, orig_stdin_flags) == -1) {
      inNonBlockingMode = false; // prevent exit(1) from being called twice
      perror("enableRawMode(): fcntl() failure");
    }
  }
}

static void enableRawMode() {
  // Enter raw mode only if STDIN is a terminal. If the input is a file or a
  // pipe (or even /dev/null), then raw mode does not make sense.
  //
  // In addition, enter raw mode only if STDOUT is also a terminal. If the
  // output is a file or a pipe, it could be piping to a pager, like the less(1)
  // program which wants to handle keyboard inputs in raw mode by itself.
  //
  // I believe this fixes https://github.com/bxparks/EpoxyDuino/issues/2 and
  // https://github.com/bxparks/EpoxyDuino/issues/25 finally.
  if (isatty(STDOUT_FILENO) && isatty(STDIN_FILENO)) {

    // Save the original config.
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
      die("enableRawMode(): tcgetattr() failure");
    }

    // The 'Enter' key in raw mode is ^M (\r, CR). But internally, we want this
    // to be ^J (\n, NL), so ICRNL and INLCR causes the ^M to become a \n.
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(/*ICRNL | INLCR |*/ INPCK | ISTRIP | IXON);

    // Set the output into cooked mode, to handle NL and CR properly.
    // Print.println() sends CR-NL (\r\n). But some code will send just \n. The
    // ONLCR translates \n into \r\n. So '\r\n' will become \r\r\n, which is
    // just fine.
    raw.c_oflag |= (OPOST | ONLCR);
    raw.c_cflag |= (CS8);

    // Enable ISIG to allow Ctrl-C to kill the program.
    raw.c_lflag &= ~(ECHO | /*ISIG |*/ ICANON | IEXTEN);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
      die("enableRawMode(): tcsetattr() failure");
    }

    inRawMode = true;
  }

  // Always set input into non-blocking mode so that yield() does not block when
  // it reads one character, emulating a read from the Serial port.
  orig_stdin_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  if (fcntl(STDIN_FILENO, F_SETFL, orig_stdin_flags | O_NONBLOCK) == -1) {
    die("enableRawMode(): fcntl() failure");
  }
  inNonBlockingMode = true;
}

// -----------------------------------------------------------------------
// Main loop. User code will provide setup() and loop().
// -----------------------------------------------------------------------

extern "C" {

int epoxy_argc;

const char* const* epoxy_argv;

static int epoxyduino_main(int argc, char** argv) {
  epoxy_argc = argc;
  epoxy_argv = argv;

  atexit(disableRawMode);
  enableRawMode();

  setup();
  while (true) {
    loop();
    yield();
  }
}

void enableTerminalEcho() {
  struct termios term;
  if (tcgetattr(STDIN_FILENO, &term) == -1) {
    return;
  }
  term.c_lflag |= ECHO;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == -1) {
    return;
  }
}

// Weak reference so that the calling code can provide its own main().
int main(int argc, char** argv)
__attribute__((weak));

int main(int argc, char** argv) {
  return epoxyduino_main(argc, argv);
}

}

#endif // #ifdef EPOXY_DUINO
