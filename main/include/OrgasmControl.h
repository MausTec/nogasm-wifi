#ifndef __OrgasmControl_h
#define __OrgasmControl_h

#include <Arduino.h>
#include "../../config.h"
#include "RunningAverage.h"
#include <SD.h>

namespace OrgasmControl {
  void tick();

  // Fetch Data
  long getArousal();
  float getArousalPercent();
  byte getMotorSpeed();
  float getMotorSpeedPercent();
  long getLastPressure();
  long getAveragePressure();
  bool updated();

  // Set Controls
  void controlMotor(bool control = true);

  // Recording Control
  void startRecording();
  void stopRecording();
  bool isRecording();

  namespace {
    long last_update_ms = 0;

    // Orgasmo Calculations
    // These can all probably be ints since we're really only using
    // 11 bits of ADC wisdom.
    RunningAverage PressureAverage;
    long last_value = 4096;
    long pressure_value = 0;
    long peak_start = 0;
    long arousal = 0;
    float motor_speed = 0;
    bool update_flag = false;
    bool control_motor = false;

    // File Writer
    long recording_start_ms = 0;
    File logfile;

    void updateArousal();
    void updateMotorSpeed();
  }
}

#endif