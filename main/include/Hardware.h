#ifndef __Hardware_h
#define __Hardware_h

#include "arduino.h"
#include "../../config.h"

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

  namespace {
    void initializeEncoder();

    void initializeButtons();

    void initializeLEDs();

    bool idle = false;
    long idle_since_ms = 0;

    int motor_speed = 0;

#ifdef LED_PIN
    CRGB leds[LED_COUNT];
#endif
    CRGB encoderColor = CRGB::Black;

    OneButton Key1(KEY_1_PIN, true, false);
    OneButton Key2(KEY_2_PIN, true, false);
    OneButton Key3(KEY_3_PIN, true, false);

    int32_t encoderCount;
    ESP32Encoder Encoder;
    OneButton EncoderSw(ENCODER_SW_PIN, false, false);
  }
}

#endif
