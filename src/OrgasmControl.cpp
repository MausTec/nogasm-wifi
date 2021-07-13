#include "OrgasmControl.h"
#include "Hardware.h"
#include "WiFiHelper.h"
#include "config.h"

namespace OrgasmControl {
  namespace {
    VibrationModeController* getVibrationMode() {
      switch (Config.vibration_mode) {
        case VibrationMode::Enhancement:
          return &VibrationControllers::Enhancement;

        case VibrationMode::Depletion:
          return &VibrationControllers::Depletion;

        case VibrationMode::Pattern:
          return &VibrationControllers::Pattern;

        default:
        case VibrationMode::RampStop:
          return &VibrationControllers::RampStop;
      }
    }

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
      long p_check = Config.use_average_values ? p_avg : pressure_value;

      // Increment arousal:
      if (p_check < last_value) { // falling edge of peak
        if (p_check > peak_start) { // first tick past peak?
          if (p_check - peak_start >= Config.sensitivity_threshold / 10) { // big peak
            arousal += p_check - peak_start;
          }
        }
        peak_start = p_check;
      }

      last_value = p_check;

      // detect muscle clenching.  (Can be used for ruined orgasm if tease threshold is set too high)
      if (p_check >= (clench_pressure_threshold + Config.clench_pressure_sensitivity) ) {
        clench_pressure_threshold = (p_check - (Config.clench_pressure_sensitivity/2)); // raise clench threshold to pressure - 1/2 sensitivity
      }
      if (p_check >= clench_pressure_threshold) {
        clench_duration += 1;   // Start counting clench time if pressure over threshold
        if ( clench_duration > Config.clench_duration_threshold) {
          arousal += 100;     // boost arousal  because clench duration exceeded
          if ( arousal > 4095 ) { arousal = 4096; } // protect arousal value to not go higher then 4096
        }
        if ( clench_duration >= (Config.clench_duration_threshold*2) ) { // desensitize clench threshold when clench too long. this is to stop arousal from going up
          clench_pressure_threshold += 400;
          clench_duration = 0;
        }
      } else {                     // when not clenching lower clench time and decay clench threshold
        clench_duration -= 5;
        if ( clench_duration <=0 ) {
          clench_duration = 0;
          if ( (p_check + (Config.clench_pressure_sensitivity/2)) < clench_pressure_threshold ){  // clench pressure threshold value decays over time to a min of pressure + 1/2 sensitivity
            clench_pressure_threshold -= 1;
          }
        }
      } // end of clenching detection
    }

    void updateMotorSpeed() {
      if (!control_motor) return;

      VibrationModeController *controller = getVibrationMode();
      controller->tick(motor_speed, arousal);

      // Calculate timeout delay
      bool time_out_over = false;
      long on_time = millis() - motor_start_time;
      if (millis() - motor_stop_time > Config.edge_delay){
        time_out_over = true;
      }

      // Ope, orgasm incoming! Stop it!
      if (!time_out_over) {
        twitchDetect();

      } else if (arousal > Config.sensitivity_threshold && motor_speed > 0 && on_time > Config.minimum_on_time) {
        // The motor_speed check above, btw, is so we only hit this once per peak.
        // Set the motor speed to 0, and set stop time.
        motor_speed = controller->stop();
        motor_stop_time = millis();
        motor_start_time = 0;
        denial_count++;

      // Start from 0
      } else if (motor_speed == 0 && motor_start_time == 0){
        motor_speed = controller->start();
        motor_start_time = millis();

      // Increment or Change
      } else {
        motor_speed = controller->increment();
      }

      // Control motor if we are not manually doing so.
      if (control_motor) {
        Hardware::setMotorSpeed(motor_speed);
      }
    }
    
    void updateEdgingTime() {
      if (!control_motor) {                   // keep edging start time to current time as long as system is in Manual Mode
        autoEdgingStartMillis = millis();
        postOrgasmStartMillis = 0;
        if (Config.sensitivity_threshold == 6000) { // set back the sensitivity_threshold if switched to manual mode before the end of the post orgasm cycle
          Config.sensitivity_threshold = original_sensitivity_threshold;
        }
        return;
      }
      VibrationModeController *controller = getVibrationMode();

      if (Config.autoEdgingDurationMinutes > 0 ) {   // Do the edging timer if not turned off
        if ( millis() > (autoEdgingStartMillis + ( Config.autoEdgingDurationMinutes * 60 * 1000 )) && postOrgasmStartMillis == 0) {  // Detect if edging time has passed
          Hardware::setEncoderColor(CRGB::Green);
          if (Config.sensitivity_threshold != 6000) {
            original_sensitivity_threshold = Config.sensitivity_threshold; // Make backup to bring back value after Post orgasm Torture
            arousal = 0;                         //make sure arousal is lower then threshold bofore starting to detect an orgasm
          }
          Config.sensitivity_threshold = 6000; // make sure orgasm is now possible
          if (arousal > original_sensitivity_threshold) { //now detect the orgasm to start post orgasm torture timer
            Hardware::setEncoderColor(CRGB::Red);
            postOrgasmStartMillis = millis();   // Start Post orgasm torture timer
          }
        } 

        if ( millis() < (postOrgasmStartMillis + (Config.postOrgasmDurationMinutes * 60 * 1000 )) && postOrgasmStartMillis >0) { // Detect if within post orgasm session
          motor_speed = Config.motor_max_speed;
        }
        if ( millis() >= (postOrgasmStartMillis + (Config.postOrgasmDurationMinutes * 60 * 1000 )) && postOrgasmStartMillis >0) { // torture until timer reached
          Hardware::setEncoderColor(CRGB::Green);
          Config.sensitivity_threshold = original_sensitivity_threshold;
          motor_speed = controller->stop();  // Turn off motor
          Hardware::setMotorSpeed(motor_speed);
          postOrgasmStartMillis = 0;  // Turn off PostEorgasm torture
          controlMotor(false);  // return to a manual mode
        }
      }
 
    }

  }

  void twitchDetect(){
    if (arousal > Config.sensitivity_threshold){
      motor_stop_time = millis();
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
      updateEdgingTime();
      updateMotorSpeed();
      update_flag = true;
      last_update_ms = millis();

      // Data for logfile or classic log.
      String data =
          String(getLastPressure()) + "," +
          String(getAveragePressure()) + "," +
          String(getArousal()) + "," +
          String(Hardware::getMotorSpeed()) + "," +
          String(Config.sensitivity_threshold) + "," +
          String(clench_pressure_threshold) + "," +
          String(clench_duration);

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

  int getDenialCount() {
    return denial_count;
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
    motor_speed = 0;
    control_motor = control;
  }

  void pauseControl() {
    prev_control_motor = control_motor;
    control_motor = false;
  }

  void resumeControl() {
    control_motor = prev_control_motor;
  }
}
