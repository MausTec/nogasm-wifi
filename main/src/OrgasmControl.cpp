#include "../include/OrgasmControl.h"
#include "../include/Hardware.h"
#include "../include/WiFiHelper.h"

namespace OrgasmControl {
  namespace {
    /**
     * Main orgasm detection / edging algorithm happens here.
     * This happens with a default update frequency of 50Hz.
     */
    void updateArousal() {
      // Decay stale arousal value:
      arousal *= 0.99;

      // Acquire new pressure and take average:
      pressure_value = Hardware::getPressure();
      PressureAverage.addValue(pressure_value);
      long p_avg = PressureAverage.getAverage();

      // Increment arousal:
      if (p_avg < last_value) { // falling edge of peak
        if (p_avg > peak_start) { // first tick past peak?
          if (p_avg - peak_start >= Config.sensitivity_threshold / 10) { // big peak
            arousal += p_avg - peak_start;
          }
        }
        peak_start = p_avg;
      }

      last_value = p_avg;
    }

    void updateMotorSpeed() {
      // Motor increment goes 0 - 100 in ramp_time_s, in steps of 1/update_fequency
      float motor_increment = (
          (float)Config.motor_max_speed /
          ((float)Config.update_frequency_hz * (float)Config.motor_ramp_time_s)
      );

      // Ope, orgasm incoming! Stop it!
      if (arousal > Config.sensitivity_threshold) {
        // Set the motor speed to 0, but actually set it to a negative number because cooldown delay
        motor_speed = max(-255.0f, -0.5f *
          (float)Config.motor_ramp_time_s *
          (float)Config.update_frequency_hz *
              motor_increment
        );

      } else if (motor_speed < Config.motor_max_speed) {
        motor_speed += motor_increment;
      }

      // Control motor if we are not manually doing so.
      if (control_motor) {
        Hardware::setMotorSpeed(motor_speed);
      }
    }
  }

  void startRecording() {
    if (logfile) {
      stopRecording();
    }

    UI.toastNow("Preapring\nrecording...", 0);

    struct tm timeinfo;
    char filename_date[16];
    if(!WiFiHelper::connected() || !getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      sprintf(filename_date, "%d", millis());
    } else {
      strftime(filename_date, 16, "%Y%m%d-%H%M%S", &timeinfo);
    }

    String logfile_name = "/log-" + String(filename_date) + ".csv";
    Serial.println("Opening logfile: " + logfile_name);
    logfile = SD.open(logfile_name, FILE_WRITE);

    if (!logfile) {
      Serial.println("Couldn't open logfile to save!" + String(logfile));
      UI.toast("Error opening\nlogfile!");
    } else {
      recording_start_ms = millis();
      logfile.println("millis,pressure,avg_pressure,arousal,motor_speed,sensitivity_threshold");
      UI.drawRecordIcon(1, 1500);
      UI.toast(String("Recording started:\n" + logfile_name).c_str());
    }
  }

  void stopRecording() {
    if (logfile) {
      UI.toastNow("Stopping...", 0);
      Serial.println("Closing logfile.");
      logfile.close();
      logfile = File();
      UI.drawRecordIcon(0);
      UI.toast("Recording stopped.");
    }
  }

  bool isRecording() {
    return (bool)logfile;
  }

  void tick() {
    long update_frequency_ms = (1.0f / Config.update_frequency_hz) * 1000.0f;

    if (millis() - last_update_ms > update_frequency_ms) {
      updateArousal();
      updateMotorSpeed();
      update_flag = true;
      last_update_ms = millis();

      // Data for logfile or classic log.
      String data =
          String(getLastPressure()) + "," +
          String(getAveragePressure()) + "," +
          String(getArousal()) + "," +
          String(Hardware::getMotorSpeed()) + "," +
          String(Config.sensitivity_threshold);

      // Write out to logfile, which includes millis:
      if (logfile) {
        logfile.println(String(last_update_ms - recording_start_ms) + "," + data);
      }

      // Write to console for classic log mode:
      if (Config.classic_serial) {
        Serial.println(data);
      }
    } else {
      update_flag = false;
    }
  }

  bool updated() {
    return update_flag;
  }

  /**
   * Returns a normalized motor speed from 0..255
   * @return normalized motor speed byte
   */
  byte getMotorSpeed() {
    return min((float)floor(max(motor_speed, 0.0f)), 255.0f);
  }

  float getMotorSpeedPercent() {
    return getMotorSpeed() / 255;
  }

  long getArousal() {
    return arousal;
  }

  float getArousalPercent() {
    return (float)arousal / Config.sensitivity_threshold;
  }

  long getLastPressure() {
    return pressure_value;
  }

  long getAveragePressure() {
    return PressureAverage.getAverage();
  }

  void controlMotor(bool control) {
    control_motor = control;
  }
}