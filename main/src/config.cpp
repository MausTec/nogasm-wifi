#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include "../../config.h"

ConfigStruct Config;

/**
 * This code loads configuration into the Config struct.
 * @see config.h
 * void loadConfigFromSd();
 */
void loadConfigFromSd() {
  StaticJsonDocument<512> doc;

  if (!SD.exists(CONFIG_FILENAME)) {
    Serial.println("Couldn't find config.json on your SD card!");
  } else {
    File configFile = SD.open(CONFIG_FILENAME);
    DeserializationError e = deserializeJson(doc, configFile);

    if (e) {
      Serial.println(F("Failed to deserialize JSON, using default config!"));
      Serial.println(F("^-- This means no WiFi. Please ensure your SD card has config.json present."));
    }
  }

  // Copy WiFi Settings
  strlcpy(Config.wifi_ssid, doc["wifi_ssid"] | "", sizeof(Config.wifi_ssid));
  strlcpy(Config.wifi_key, doc["wifi_key"] | "", sizeof(Config.wifi_key));
  Config.wifi_on = doc["wifi_on"] | false;

  // Copy Bluetooth Settings
  strlcpy(Config.bt_display_name, doc["bt_display_name"] | "", sizeof(Config.bt_display_name));
  Config.bt_on = doc["bt_on"] | false;

  // Copy Network Settings
  Config.websocket_port = doc["websocket_port"] | 80;
  Config.classic_serial = doc["classic_serial"] | true;

  // Copy UI Settings
  Config.led_brightness = doc["led_brightness"] | 128;
  Config.screen_dim_seconds = doc["screen_dim_seconds"] | 10;

  // Copy Orgasm Settings
  Config.motor_max_speed = doc["motor_max_speed"] | 128;
  Config.pressure_smoothing = doc["pressure_smoothing"] | 5;
  Config.sensitivity_threshold = doc["sensitivity_threshold"] | 600;
  Config.motor_ramp_time_s = doc["motor_ramp_time_s"] | 30;
  Config.update_frequency_hz = doc["update_frequency_hz"] | 50;
  Config.sensor_sensitivity = doc["sensor_sensitivity"] | 128;
}

void saveConfigToSd(long save_at_ms) {
  static long save_at_ms_tick = 0;

  if (save_at_ms > 0) {
    // Queue a future save:
    Serial.println("Future save queued.");
    save_at_ms_tick = max(save_at_ms, save_at_ms_tick);
    return;
  } else if (save_at_ms < 0) {
    if (save_at_ms_tick > 0 && save_at_ms_tick < millis()) {
      Serial.println("Saving now from future save queue...");
      save_at_ms_tick = 0;
    } else {
      return;
    }
  }

  StaticJsonDocument<512> doc;

  SD.remove(CONFIG_FILENAME ".bak");
  if (!SD.rename(CONFIG_FILENAME, CONFIG_FILENAME ".bak")) {
    Serial.println(F("Failed to save over existing config!"));
    return;
  }

  SD.remove(CONFIG_FILENAME);
  File tmp = SD.open(CONFIG_FILENAME, FILE_WRITE);
  if (!tmp) {
    Serial.println(F("Failed to create temp file for config save!"));
    return;
  }

  // Copy WiFi Settings
  doc["wifi_ssid"] = Config.wifi_ssid;
  doc["wifi_key"] = Config.wifi_key;
  doc["wifi_on"] = Config.wifi_on;

  // Copy Bluetooth Settings
  doc["bt_display_name"] = Config.bt_display_name;
  doc["bt_on"] = Config.bt_on;

  // Copy Network Settings
  doc["websocket_port"] = Config.websocket_port;
  doc["classic_serial"] = Config.classic_serial;

  // Copy UI Settings
  doc["led_brightness"] = Config.led_brightness;
  doc["screen_dim_seconds"] = Config.screen_dim_seconds;

  // Copy Orgasm Settings
  doc["motor_max_speed"] = Config.motor_max_speed;
  doc["pressure_smoothing"] = Config.pressure_smoothing;
  doc["sensitivity_threshold"] = Config.sensitivity_threshold;
  doc["motor_ramp_time_s"] = Config.motor_ramp_time_s;
  doc["update_frequency_hz"] = Config.update_frequency_hz;
  doc["sensor_sensitivity"] = Config.sensor_sensitivity;


  // Serialize and move temp file
  if (serializeJson(doc, tmp) == 0) {
    Serial.println(F("Failed to serialize config to file!"));
    tmp.close();
  }

  tmp.close();
}