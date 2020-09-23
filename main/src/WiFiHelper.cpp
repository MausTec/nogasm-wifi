#include "../include/WiFiHelper.h"

#include "../include/UserInterface.h"
#include <WiFi.h>

namespace WiFiHelper {
  void drawSignalIcon() {
    UI.drawWifiIcon(getWiFiStrength() + 1);
  }

  bool connected() {
    return WiFi.status() == WL_CONNECTED;
  }

  String ip() {
    if (connected()) {
      return WiFi.localIP().toString();
    } else {
      return "";
    }
  }

  bool begin() {
    if (!Config.wifi_on) {
      return false;
    }

    // Connect to access point
    Serial.print("Connecting..");
    WiFi.begin(Config.wifi_ssid, Config.wifi_key);
    UI.display->setCursor(0,0);
    int count = 0;
    long connection_start_at_ms = millis();
    while ( WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      if (millis() - connection_start_at_ms > CONNECTION_TIMEOUT_S * 1000) {
        Serial.println();
        Serial.print("Connection timed out!");
        return false;
      }
      delay(500);
    }

    // Print our IP address
    Serial.println(" Connected!");
    Serial.print("My IP address: ");
    Serial.println(WiFi.localIP());

    // Synchronize Local Clock
    const char* ntpServer = "pool.ntp.org";
    const long  gmtOffset_sec = 0;
    const int   daylightOffset_sec = 3600;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // Time Example TODO-Remove
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return false;
    }

    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    return true;
  }

  String signalStrengthStr() {
    switch(getWiFiStrength()) {
      case 0:
        return "No Signal";
      case 1:
        return "Weak";
      case 2:
        return "Okay";
      case 3:
        return "Strong";
      default:
        return "Unknown";
    }
  }

  void disconnect() {
    WiFi.disconnect(true);
  }

  namespace {
    byte getWiFiStrength() {
      byte wifiStrength;
      int rssi = WiFi.RSSI();

      if (rssi < -90) {
        wifiStrength = 0;
      } else if (rssi < -80) {
        wifiStrength = 1;
      } else if (rssi < -60) {
        wifiStrength = 2;
      } else if (rssi < 0) {
        wifiStrength = 3;
      } else {
        wifiStrength = 0;
      }

      return wifiStrength;
    }
  }
}