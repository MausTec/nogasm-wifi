#ifndef __WiFiHelper_h
#define __WiFiHelper_h

#include "Arduino.h"

#define CONNECTION_TIMEOUT_S 5

namespace WiFiHelper {
  namespace {
    byte getWiFiStrength();
  }

  bool begin();
  bool connected();
  void disconnect();
  String ip();
  String signalStrengthStr();
  void drawSignalIcon();
}

#endif