/*
 *  Udp.cpp: Library to send/receive UDP packets.
 *
 * NOTE: UDP is fast, but has some important limitations (thanks to Warren Gray for mentioning these)
 * 1) UDP does not guarantee the order in which assembled UDP packets are received. This
 * might not happen often in practice, but in larger network topologies, a UDP
 * packet can be received out of sequence.
 * 2) UDP does not guard against lost packets - so packets *can* disappear without the sender being
 * aware of it. Again, this may not be a concern in practice on small local networks.
 * For more information, see http://www.cafeaulait.org/course/week12/35.html
 *
 * MIT License:
 * Copyright (c) 2008 Bjoern Hartmann
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * bjoern@cs.stanford.edu 12/30/2008
 *
 * Modified for EpoxyDuino by Brian Park 2022:
 *  * AVR version comes from version 1.8.5 of
 *    https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/Udp.h
 *  * ESP8226 version comes from version 3.0.2 of
 *    https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Udp.h
 */

#ifndef EPOXY_DUINO_UDP_H
#define EPOXY_DUINO_UDP_H

#include <Stream.h>
#include <IPAddress.h>

class UDP : public Stream {
  public:
  #if defined(EPOXY_CORE_ESP8266)
    virtual ~UDP() {};
  #endif

    virtual uint8_t begin(uint16_t) =0;
  #if defined(EPOXY_CORE_AVR)
    virtual uint8_t beginMulticast(IPAddress, uint16_t) { return 0; }
  #endif
    virtual void stop() =0;

    virtual int beginPacket(IPAddress ip, uint16_t port) =0;
    virtual int beginPacket(const char *host, uint16_t port) =0;
    virtual int endPacket() =0;
    virtual size_t write(uint8_t) =0;
    virtual size_t write(const uint8_t *buffer, size_t size) =0;

    virtual int parsePacket() =0;
    virtual int available() =0;
    virtual int read() =0;
    virtual int read(unsigned char* buffer, size_t len) =0;
    virtual int read(char* buffer, size_t len) =0;
    virtual int peek() =0;
    virtual void flush() =0;

    virtual IPAddress remoteIP() =0;
    virtual uint16_t remotePort() =0;

  protected:
    uint8_t* rawIPAddress(IPAddress& addr) {
      return addr.raw_address();
    };

  #if defined(EPOXY_CORE_ESP8266)
    const uint8_t* rawIPAddress(const IPAddress& addr) {
        return addr.raw_address();
    }
  #endif
};

#endif
