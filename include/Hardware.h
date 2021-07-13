#ifndef __Hardware_h
#define __Hardware_h

#include "config.h"

#include "UserInterface.h"

#include <FastLED.h>
#include <OneButton.h>
#include <ESP32Encoder.h>
#include <analogWrite.h>

namespace Hardware {
  bool initialize();
  void tick();

  void setEncoderColor(CRGB color);
  void setLedColor(byte i, CRGB color = CRGB::Black);
  void ledShow();

  void setMotorSpeed(int speed);
  void changeMotorSpeed(int diff);
  int getMotorSpeed();
  float getMotorSpeedPercent();

  long getPressure();
  void setPressureSensitivity(byte value);
  /**
   * Gets the pressure sensitivity setting from the digitpot.
   * @return current digipot wiper position (1-127)
   */
  byte getPressureSensitivity();

  void enableExternalBus();
  void disableExternalBus();
  void joinI2c(byte address);
  void leaveI2c();
  void handleI2c(int avail);

  String getDeviceSerial();
  void setDeviceSerial(const char *serial);

  namespace {
    void initializeEncoder();

    void initializeButtons();

    void initializeLEDs();

    bool idle = false;
    bool standby = false;
    long idle_since_ms = 0;

    int motor_speed = 0;

    byte i2c_slave_addr = 0;
    bool external_connected = false;

#ifdef LED_PIN
    CRGB leds[LED_COUNT];
#endif
    CRGB encoderColor = CRGB::Black;

#ifdef KEY_1_PIN
    OneButton Key1(KEY_1_PIN, true, false);
    OneButton Key2(KEY_2_PIN, true, false);
    OneButton Key3(KEY_3_PIN, true, false);
#endif

    int32_t encoderCount;
    ESP32Encoder Encoder;
    OneButton EncoderSw(ENCODER_SW_PIN, false, false);
  }
}

#endif
