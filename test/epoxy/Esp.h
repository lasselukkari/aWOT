#ifndef EPOXY_DUINO_ESP_H
#define EPOXY_DUINO_ESP_H

#if defined(EPOXY_CORE_ESP8266)

#include <sys/time.h>
#include "Stream.h"

class EspClass
{
  public:
    EspClass() {
      gettimeofday(&start, NULL);
    }

    void reset() {}

    void restart() {}

    // Very ugly approximation, this is freeStack
    unsigned long getFreeHeap() {
      int i;
      static int* h=40000+&i;
      return h-&i;
    }

    uint32_t getCpuFreqMHZ() { return 80; }

    uint32_t getChipId() { return 0; }

    uint32_t getCycleCount() {
      struct timeval now;
      gettimeofday(&now, NULL);
      return getCpuFreqMHZ()
          * ((now.tv_sec-start.tv_sec)*1000000+(now.tv_usec-start.tv_usec));
    }

  private:
    struct timeval start;
};

class Serial_ : public Stream
{
};

extern EspClass ESP;

#endif
#endif
