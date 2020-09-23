#include "../include/Hardware.h"
#include "../include/OrgasmControl.h"

#include <Wire.h>

namespace Hardware {
  bool initialize() {
    initializeButtons();
    initializeEncoder();
    initializeLEDs();

#ifndef LED_PIN
    pinMode(RJ_LED_1_PIN, OUTPUT);
    pinMode(RJ_LED_2_PIN, OUTPUT);
    digitalWrite(RJ_LED_1_PIN, LOW);
    digitalWrite(RJ_LED_2_PIN, LOW);
#endif

    Wire.begin();
    setPressureSensitivity(Config.sensor_sensitivity);

    return true;
  }

  void tick() {
    Key1.tick();
    Key2.tick();
    Key3.tick();

    EncoderSw.tick();

    // Debug Encoder:
    int32_t count = Encoder.getCount();
    if (count != encoderCount) {
      idle_since_ms = millis();
      UI.onEncoderChange(count - encoderCount);
      encoderCount = count;
    }

    if (Config.screen_dim_seconds > 0) {
      if ((millis() - idle_since_ms) > Config.screen_dim_seconds * 1000) {
        if (!idle) {
          UI.display->dim(true);
          idle = true;
        }
      } else {
        if (idle) {
          UI.display->dim(false);
          idle = false;
        }
      }
    }
  }

  void setLedColor(byte i, CRGB color) {
#ifdef LED_PIN
    leds[i] = color;
#endif
  }

  void setEncoderColor(CRGB color) {
    encoderColor = color;
    analogWrite(ENCODER_RD_PIN, color.r);
    analogWrite(ENCODER_GR_PIN, color.g);
    analogWrite(ENCODER_BL_PIN, color.b);
  }

  void setMotorSpeed(int speed) {
    int new_speed = min(max(speed, 0), 255);
    if (new_speed == motor_speed) return;

    motor_speed = new_speed;
    analogWrite(MOT_PWM_PIN, motor_speed);
  }

  void changeMotorSpeed(int diff) {
    int new_speed = motor_speed + diff;
    setMotorSpeed(new_speed);
  }

  int getMotorSpeed() {
    return motor_speed;
  }

  float getMotorSpeedPercent() {
    return (float)motor_speed / 255.0;
  }

  void ledShow() {
#ifdef LED_PIN
    FastLED.show();
#endif
  }

  long getPressure() {
    return analogRead(BUTT_PIN);
  }

  void setPressureSensitivity(byte value) {
    Wire.beginTransmission(0x2F);
    Wire.write((byte)(255 - value) / 2);
    Wire.endTransmission();
  }

  namespace {
    void initializeButtons() {
      Key1.attachClick([]() {
        idle_since_ms = millis();
        UI.onKeyPress(0);
      });

      Key1.attachPress([]() {
        if (OrgasmControl::isRecording()) {
          OrgasmControl::stopRecording();
        } else {
          OrgasmControl::startRecording();
        }
      });

      Key2.attachClick([]() {
        idle_since_ms = millis();
        UI.onKeyPress(1);
      });

      Key3.attachClick([]() {
        idle_since_ms = millis();
        UI.onKeyPress(2);
      });
    }

    void initializeEncoder() {
      pinMode(ENCODER_RD_PIN, OUTPUT);
      pinMode(ENCODER_GR_PIN, OUTPUT);
      pinMode(ENCODER_BL_PIN, OUTPUT);

      setEncoderColor(CRGB::Black);

      ESP32Encoder::useInternalWeakPullResistors = UP;
      Encoder.attachSingleEdge(ENCODER_A_PIN, ENCODER_B_PIN);
      Encoder.setCount(128);
      encoderCount = 128;

      EncoderSw.attachClick([]() {
        idle_since_ms = millis();
        UI.onKeyPress(3);
      });

      // TODO: This should be EncoderSw
      EncoderSw.attachPress([]() {
        UI.screenshot();
      });
    }

    void initializeLEDs() {
#ifdef LED_PIN
      pinMode(LED_PIN, OUTPUT);

      FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);
      for (int i = 0; i < LED_COUNT; i++) {
        leds[i] = CRGB::Black;
      }
      FastLED.show();
#endif
    }
  }
}