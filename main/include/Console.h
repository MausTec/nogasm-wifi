#ifndef __Console_h
#define __Console_h

#include "Arduino.h"

#define SERIAL_BUFFER_LEN 256

namespace Console {
  void loop();

  namespace {
    void handleMessage();

    char buffer[SERIAL_BUFFER_LEN] = {0};
    size_t buffer_i = 0;
  }
}

#endif