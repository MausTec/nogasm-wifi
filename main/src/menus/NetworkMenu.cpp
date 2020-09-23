#include "../../include/UIMenu.h"
#include "../../include/UserInterface.h"
#include "../../include/WiFiHelper.h"

#include <WiFi.h>

static void onDisableWiFi(UIMenu *menu) {
  UI.toastNow("Disconnecting...", 0, false);
  Config.wifi_on = false;
  WiFiHelper::disconnect();
  saveConfigToSd(0);
  UI.toast("Disconnected.", 3000);
  menu->initialize();
  menu->render();
}

static void onEnableWiFi(UIMenu *menu) {
  if (Config.wifi_ssid[0] == '\0' || Config.wifi_key[0] == '\0') {
    UI.toastNow("No WiFi Config\nEdit config.json\non SD card.", 0);
    return;
  }

  UI.toastNow("Connecting...", 0, false);
  Config.wifi_on = true;
  if (WiFiHelper::begin()) {
    UI.toastNow("WiFi Connected!", 3000);
    saveConfigToSd(0);
    menu->initialize();
  } else {
    UI.toastNow("Failed to connect.", 3000);
    Config.wifi_on = false;
  }

  menu->render();
}

static void onViewStatus(UIMenu*) {
  String status = "";

  if (WiFiHelper::connected()) {
    status += "Connected";
    status += "\n" + WiFiHelper::ip();
    status += "\nSignal: " + WiFiHelper::signalStrengthStr();
  } else {
    status += "Disconnected";
  }

  UI.toast(status.c_str(), 0);
}

static void buildMenu(UIMenu *menu) {
  if (WiFiHelper::connected()) {
    menu->addItem("Disable WiFi", &onDisableWiFi);
  } else {
    menu->addItem("Enable WiFi", &onEnableWiFi);
  }

  menu->addItem("Connection Status", &onViewStatus);
}

UIMenu NetworkMenu("WiFi Settings", &buildMenu);