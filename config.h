#ifndef __config_h
#define __config_h

#include "arduino.h"
#include "errors.h"

#include <ArduinoJson.h>

// External Library Config
#define FASTLED_INTERNAL

// SD card files:
#define CONFIG_FILENAME "/config.json"
#define UPDATE_FILENAME "/update.bin"

// Uncomment to enable debug logging and functions.
#define DEBUG

// Uncomment if compiling for NoGasm+
//#define NG_PLUS

// Butt Pin
#define BUTT_PIN        34
#define MOT_PWM_PIN     15

// SD Connections
#define SD_CS_PIN       5

// Encoder Connection
#define ENCODER_B_PIN   32
#define ENCODER_A_PIN   33
#define ENCODER_SW_PIN  35
#define ENCODER_RD_PIN  2
#define ENCODER_BL_PIN  27
#define ENCODER_GR_PIN  4

#ifdef NG_PLUS
  // LCD Definitions
  #define SCREEN_WIDTH 128
  #define SCREEN_HEIGHT 32

  // LED Ring (NoGasm+)
  #define LED_PIN 17
  #define LED_COUNT 13

  #define SDA_PIN 21
  #define SCL_PIN 22

  // Encoders are swapped
  #define ENCODER_GR_PIN 27
  #define ENCODER_BL_PIN 4
#else
  // LCD Definitions
  #define SCREEN_WIDTH    128
  #define SCREEN_HEIGHT   64
  #define OLED_DC         13
  #define OLED_RESET      14
  #define OLED_CS         12

  // External Bus (NoGasm WiFi)
  #define BUS_EN_PIN      20
  #define RJ_LED_1_PIN    17
  #define RJ_LED_2_PIN    16
  #define SDA_PIN 21
  #define SCL_PIN 22
  #define I2C_SLAVE_ADDR 0x09

  // Buttons
  #define KEY_1_PIN       39
  #define KEY_2_PIN       25
  #define KEY_3_PIN       26
#endif

union ConfigValue {
  byte v_byte;
  int  v_int;
  char *v_char;
  bool v_bool;
};

struct ConfigStruct {
  // Networking
  char wifi_ssid[64];
  char wifi_key[64];
  bool wifi_on;

  char bt_display_name[64];
  bool bt_on;

  // UI And Stuff
  byte led_brightness;
  int screen_dim_seconds;
  int screen_timeout_seconds;

  // Server
  int websocket_port;
  bool classic_serial;
  bool use_ssl;
  char hostname[64];

  // Orgasms and Stuff
  byte motor_max_speed;
  byte motor_start_speed;
  int edge_delay;
  int max_additional_delay;
  int minimum_on_time;
  byte pressure_smoothing;
  int sensitivity_threshold;
  int motor_ramp_time_s;
  int update_frequency_hz;
  byte sensor_sensitivity;
  bool use_average_values;
} extern Config;

extern void loadConfigFromSd();
extern void loadDefaultConfig();
extern void loadConfigFromJsonObject(JsonDocument &doc);
extern void saveConfigToSd(long save_at_ms = -1);
extern bool setConfigValue(const char *key, const char *value, bool &require_reboot);
extern bool getConfigValue(const char *key, String &out);
extern void dumpConfigToJsonObject(JsonDocument &doc);
extern bool dumpConfigToJson(String &str);
#endif