#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include "config.h"
#include "UserInterface.h"
#include "Hardware.h"
#include "Page.h"

#include <FastLed.h>

ConfigStruct Config;

/**
 * Cast a string to a bool. Accepts "false", "no", "off", "0" to false, all other
 * strings cast to true.
 * @param a
 * @return
 */
bool atob(const char *a) {
//  for (int i = 0; a[i]; i++) {
//    a[i] = tolower(a[i]);
//  }

  return !(
      strcmp(a, "false") == 0 ||
      strcmp(a, "no") == 0 ||
      strcmp(a, "off") == 0 ||
      strcmp(a, "0") == 0
  );
}

/**
 * This code loads configuration into the Config struct.
 * @see config.h
 * void loadConfigFromSd();
 */
void loadConfigFromSd() {
  DynamicJsonDocument doc(2048);

  if (!SD.exists(CONFIG_FILENAME)) {
    Serial.println(F("Couldn't find config.json on your SD card!"));
    loadConfigFromJsonObject(doc);
    saveConfigToSd(0);
  } else {
    File configFile = SD.open(CONFIG_FILENAME);
    DeserializationError e = deserializeJson(doc, configFile);

    if (e) {
      Serial.println(F("Failed to deserialize JSON, using default config!"));
      Serial.println(F("^-- This means no WiFi. Please ensure your SD card has config.json present."));
      Serial.println(e.c_str());
    }

    loadConfigFromJsonObject(doc);
  }
}

/**
 * This code loads default config, which is useful if the SD card was not
 * mounted, as it executes no SD commands.
 */
void loadDefaultConfig() {
  DynamicJsonDocument doc(1);
  loadConfigFromJsonObject(doc);
}

void loadConfigFromJsonObject(JsonDocument &doc) {
  // Copy WiFi Settings
  strlcpy(Config.wifi_ssid, doc["wifi_ssid"] | "", sizeof(Config.wifi_ssid));
  strlcpy(Config.wifi_key, doc["wifi_key"] | "", sizeof(Config.wifi_key));
  Config.wifi_on = doc["wifi_on"] | false;

  // Copy Bluetooth Settings
  strlcpy(Config.bt_display_name, doc["bt_display_name"] | "Edge-o-Matic 3000", sizeof(Config.bt_display_name));
  Config.bt_on = doc["bt_on"] | false;

  // Copy Network Settings
  Config.websocket_port = doc["websocket_port"] | 80;
  Config.classic_serial = doc["classic_serial"] | false;
  Config.use_ssl = doc["use_ssl"] | false;
  strlcpy(Config.hostname, doc["hostname"] | "eom3k", sizeof(Config.hostname));

  // Copy UI Settings
  Config.led_brightness = doc["led_brightness"] | 128;
  Config.screen_dim_seconds = doc["screen_dim_seconds"] | 10;
  Config.screen_timeout_seconds = doc["screen_timeout_seconds"] | 0;

  // Copy Orgasm Settings
  Config.motor_max_speed = doc["motor_max_speed"] | 128;
  Config.motor_start_speed = doc["motor_start_speed"] | 10;
  Config.edge_delay = doc["edge_delay"] | 10000;
  Config.minimum_on_time = doc["minimum_on_time"] | 1000;
  Config.pressure_smoothing = doc["pressure_smoothing"] | 5;
  Config.sensitivity_threshold = doc["sensitivity_threshold"] | 600;
  Config.motor_ramp_time_s = doc["motor_ramp_time_s"] | 30;
  Config.update_frequency_hz = doc["update_frequency_hz"] | 50;
  Config.sensor_sensitivity = doc["sensor_sensitivity"] | 128;
  Config.use_average_values = doc["use_average_values"] | false;

  // Copy Vibration Settings
  Config.vibration_mode = (VibrationMode)(doc["vibration_mode"] | (int)VibrationMode::RampStop);
  
  // Clench settings
  Config.clench_pressure_sensitivity = doc["clench_pressure_sensitivity"] | 200;
  Config.clench_duration_threshold = doc["clench_duration_threshold"] | 35;
  Config.autoEdgingDurationMinutes = doc["auto_edging_duration_minutes"] | 30;
  Config.postOrgasmDurationMinutes = doc["post_Orgasm_Duration_Minutes"] | 1;

  /**
   * Setting Validations
   */

  // Bluetooth and WiFi cannot operate at the same time.
  if (Config.wifi_on && Config.bt_on) {
    Serial.println("Not enough memory for WiFi and Bluetooth, disabling BT.");
    Config.bt_on = false;
  }
} // loadConfigFromJsonObject

void dumpConfigToJsonObject(JsonDocument &doc) {
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
  doc["use_ssl"] = Config.use_ssl;
  doc["hostname"] = Config.hostname;

  // Copy UI Settings
  doc["led_brightness"] = Config.led_brightness;
  doc["screen_dim_seconds"] = Config.screen_dim_seconds;
  doc["screen_timeout_seconds"] = Config.screen_timeout_seconds;

  // Copy Orgasm Settings
  doc["motor_max_speed"] = Config.motor_max_speed;
  doc["motor_start_speed"] = Config.motor_start_speed;
  doc["edge_delay"] = Config.edge_delay;
  doc["minimum_on_time"] = Config.minimum_on_time;
  doc["pressure_smoothing"] = Config.pressure_smoothing;
  doc["sensitivity_threshold"] = Config.sensitivity_threshold;
  doc["motor_ramp_time_s"] = Config.motor_ramp_time_s;
  doc["update_frequency_hz"] = Config.update_frequency_hz;
  doc["sensor_sensitivity"] = Config.sensor_sensitivity;
  doc["use_average_values"] = Config.use_average_values;

  // Vibration Settings
  doc["vibration_mode"] = (int) Config.vibration_mode;
  
  // Clench settings
  doc["clench_pressure_sensitivity"] = Config.clench_pressure_sensitivity;
  doc["clench_duration_threshold"] = Config.clench_duration_threshold;
  doc["auto_edging_duration_minutes"] = Config.autoEdgingDurationMinutes;
  doc["post_Orgasm_Duration_Minutes"] = Config.postOrgasmDurationMinutes;
} // dumpConfigToJsonObject

bool dumpConfigToJson(String &str) {
  DynamicJsonDocument doc(2048);
  dumpConfigToJsonObject(doc);

  // Serialize and move temp file
  if (serializeJsonPretty(doc, str) == 0) {
    Serial.println(F("Failed to serialize config!"));
    return false;
  }

  return true;
}

void saveConfigToSd(long save_at_ms) {
  static long save_at_ms_tick = 0;
  String config;

  if (save_at_ms > 0) {
    // Queue a future save:
    save_at_ms_tick = max(save_at_ms, save_at_ms_tick);
    return;
  } else if (save_at_ms < 0) {
    if (save_at_ms_tick > 0 && save_at_ms_tick < millis()) {
      Serial.println(F("Saving now from future save queue..."));
      save_at_ms_tick = 0;
    } else {
      return;
    }
  }

  WebSocketHelper::sendSettings();

  if (SD.exists(CONFIG_FILENAME)) {
    SD.remove(CONFIG_FILENAME ".bak");
    if (!SD.rename(CONFIG_FILENAME, CONFIG_FILENAME ".bak")) {
      Serial.println(F("Failed to save over existing config!"));
      UI.toast("Error " E_SAV_BAK);
      return;
    }
  }

  File tmp = SD.open(CONFIG_FILENAME, FILE_WRITE);
  if (!tmp) {
    Serial.println(F("Failed to create temp file for config save!"));
    UI.toast("Error " E_SAV_TMP);
    return;
  }

  // Serialize and move temp file
  if (!dumpConfigToJson(config)) {
    Serial.println(F("Failed to serialize config to file!"));
    UI.toast("Error " E_SAV_SER);
    tmp.close();
  }

  tmp.print(config);
  tmp.close();
}

bool setConfigValue(const char *option, const char *value, bool &require_reboot) {
  if (!strcmp(option, "wifi_on")) {
    Config.wifi_on = atob(value);
    require_reboot = true;
  } else if(!strcmp(option, "bt_on")) {
    Config.bt_on = atob(value);
    require_reboot = true;
  } else if(!strcmp(option, "led_brightness")) {
    Config.led_brightness = atoi(value);
  } else if(!strcmp(option, "websocket_port")) {
    Config.websocket_port = atoi(value);
    require_reboot = true;
  } else if(!strcmp(option, "motor_max_speed")) {
    Config.motor_max_speed = atoi(value);
  } else if(!strcmp(option, "motor_start_speed")) {
    Config.motor_start_speed = atoi(value);
  } else if(!strcmp(option, "edge_delay")) {
    Config.edge_delay = atoi(value);
  } else if(!strcmp(option, "minimum_on_time")) {
    Config.minimum_on_time = atoi(value);
  } else if(!strcmp(option, "screen_dim_seconds")) {
    Config.screen_dim_seconds = atoi(value);
  } else if(!strcmp(option, "screen_timeout_seconds")) {
    Config.screen_timeout_seconds = atoi(value);
  } else if(!strcmp(option, "pressure_smoothing")) {
    Config.pressure_smoothing = atoi(value);
  } else if(!strcmp(option, "classic_serial")) {
    Config.classic_serial = atob(value);
  } else if(!strcmp(option, "use_average_values")) {
    Config.use_average_values = atob(value);
  } else if(!strcmp(option, "sensitivity_threshold")) {
    Config.sensitivity_threshold = atoi(value);
  } else if(!strcmp(option, "motor_ramp_time_s")) {
    Config.motor_ramp_time_s = atoi(value);
  } else if(!strcmp(option, "update_frequency_hz")) {
    Config.update_frequency_hz = atoi(value);
  } else if(!strcmp(option, "sensor_sensitivity")) {
    Config.sensor_sensitivity = atoi(value);
    Hardware::setPressureSensitivity(Config.sensor_sensitivity);
  } else if (!strcmp(option, "knob_rgb")) {
    uint32_t color = strtoul(value, NULL, 16);
    Hardware::setEncoderColor(CRGB(color));
  } else if (!strcmp(option, "wifi_ssid")) {
    strlcpy(Config.wifi_ssid, value, sizeof(Config.wifi_ssid));
  } else if (!strcmp(option, "wifi_key")) {
    strlcpy(Config.wifi_key, value, sizeof(Config.wifi_key));
  } else if (!strcmp(option, "bt_display_name")) {
    strlcpy(Config.bt_display_name, value, sizeof(Config.bt_display_name));
  } else if (!strcmp(option, "use_ssl")) {
    Config.use_ssl = atob(value);
    require_reboot = true;
  } else if (!strcmp(option, "hostname")) {
    strlcpy(Config.hostname, value, sizeof(Config.hostname));
    require_reboot = true;
  } else if (!strcmp(option, "vibration_mode")) {
    Config.vibration_mode = (VibrationMode) atoi(value);
  } else if(!strcmp(option, "clench_pressure_sensitivity")) {
    Config.clench_pressure_sensitivity = atoi(value);
  } else if(!strcmp(option, "clench_duration_threshold")) {
    Config.clench_duration_threshold = atoi(value);
  } else if(!strcmp(option, "auto_edging_duration_minutes")) {
    Config.autoEdgingDurationMinutes = atoi(value);
  } else if(!strcmp(option, "post_Orgasm_Duration_Minutes")) {
    Config.postOrgasmDurationMinutes = atoi(value);
  } else {
    return false;
  }

  return true;
} // setConfigValue

bool getConfigValue(const char *option, String &out) {
  if (!strcmp(option, "wifi_on")) {
    out += String(Config.wifi_on) + '\n';
  } else if(!strcmp(option, "bt_on")) {
    out += String(Config.bt_on) + '\n';
  } else if(!strcmp(option, "led_brightness")) {
    out += String(Config.led_brightness) + '\n';
  } else if(!strcmp(option, "websocket_port")) {
    out += String(Config.websocket_port) + '\n';
  } else if(!strcmp(option, "motor_max_speed")) {
    out += String(Config.motor_max_speed) + '\n';
  } else if(!strcmp(option, "motor_start_speed")) {
    out += String(Config.motor_start_speed) + '\n';
  } else if(!strcmp(option, "edge_delay")) {
    out += String(Config.edge_delay) + '\n';
  } else if(!strcmp(option, "minimum_on_time")) {
    out += String(Config.minimum_on_time) + '\n';
  } else if(!strcmp(option, "screen_dim_seconds")) {
    out += String(Config.screen_dim_seconds) + '\n';
  } else if(!strcmp(option, "screen_timeout_seconds")) {
    out += String(Config.screen_timeout_seconds) + '\n';
  } else if(!strcmp(option, "pressure_smoothing")) {
    out += String(Config.pressure_smoothing) + '\n';
  } else if(!strcmp(option, "classic_serial")) {
    out += String(Config.classic_serial) + '\n';
  } else if(!strcmp(option, "use_average_values")) {
    out += String(Config.use_average_values) + '\n';
  } else if(!strcmp(option, "sensitivity_threshold")) {
    out += String(Config.sensitivity_threshold) + '\n';
  } else if(!strcmp(option, "motor_ramp_time_s")) {
    out += String(Config.motor_ramp_time_s) + '\n';
  } else if(!strcmp(option, "update_frequency_hz")) {
    out += String(Config.update_frequency_hz) + '\n';
  } else if(!strcmp(option, "sensor_sensitivity")) {
    out += String(Config.sensor_sensitivity) + '\n';
  } else if (!strcmp(option, "knob_rgb")) {
    out += ("Usage: set knob_rgb 0xFFCCAA") + '\n';
  } else if (!strcmp(option, "wifi_ssid")) {
    out += String(Config.wifi_ssid) + '\n';
  } else if (!strcmp(option, "wifi_key")) {
    out += String(Config.wifi_key) + '\n';
  } else if (!strcmp(option, "bt_display_name")) {
    out += String(Config.bt_display_name) + '\n';
  } else if (!strcmp(option, "use_ssl")) {
    out += String(Config.use_ssl) + '\n';
  } else if (!strcmp(option, "hostname")) {
    out += String(Config.hostname) + '\n';
  } else if (!strcmp(option, "vibration_mode")) {
    out += String((int) Config.vibration_mode) + '\n';
  } else if(!strcmp(option, "clench_pressure_sensitivity")) { 
    out += String(Config.clench_pressure_sensitivity) + '\n';
  } else if(!strcmp(option, "clench_duration_threshold")) {
    out += String(Config.clench_duration_threshold) + '\n';
  } else if(!strcmp(option, "auto_edging_duration_minutes")) {
    out += String(Config.autoEdgingDurationMinutes) + '\n';
  } else if(!strcmp(option, "post_Orgasm_Duration_Minutes")) {
    out += String(Config.postOrgasmDurationMinutes) + '\n';
  } else {
    return false;
  }

  return true;
} // getConfigValue
