/* Linux uses the same POSIX socket implementation as the macOS example. */

#ifndef LINUXSOCKET_H_
#define LINUXSOCKET_H_

#include "../MacOSNative/MacOSSocket.h"

using LinuxClient = MacOSClient;
using LinuxServer = MacOSServer;

#endif
