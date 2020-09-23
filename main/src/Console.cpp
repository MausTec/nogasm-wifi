#include "../include/Console.h"
#include "../include/Hardware.h"
#include "../../config.h"

typedef int (*cmd_func)(char**);

typedef struct Command {
  char *cmd;
  char *alias;
  char *help;
  cmd_func func;
} Command;

/**
 * Cast a string to a bool. Accepts "false", "no", "off", "0" to false, all other
 * strings cast to true.
 * @param a
 * @return
 */
bool atob(char *a) {
  for (int i = 0; a[i]; i++) {
    a[i] = tolower(a[i]);
  }

  return !(
      strcmp(a, "false") == 0 ||
      strcmp(a, "no") == 0 ||
      strcmp(a, "off") == 0 ||
      strcmp(a, "0") == 0
  );
}

namespace Console {
  namespace {
    int sh_help(char **args);
    int sh_set(char **args);
    int sh_bool(char **args);

    Command commands[] = {
      {
        .cmd = "help",
        .alias = "?",
        .help = "Get help, if you need it.",
        .func = &sh_help
      },
      {
        .cmd = "set",
        .alias = "s",
        .help = "Set a config value",
        .func = &sh_set
      },
      {
        .cmd = "bool",
        .alias = "b",
        .help = "Test the bool cast",
        .func = &sh_bool
      }
    };

    int sh_help(char **args) {
      for (int i = 0; i < sizeof(commands)/sizeof(Command); i++) {
        Command c = commands[i];
        Serial.println(String(c.cmd) + "\t(" + String(c.alias) + ")\t" + String(c.help));
      }
    }

    int sh_set(char **args) {
      char *option = args[0];
      bool require_reboot = false;

      if (option == NULL) {
        Serial.println("An option is required.");
        return 1;
      }

      // Go through boolean + int values first:
      if (!strcmp(option, "wifi_on")) {
        if (!args[1]) {
          Serial.println(String(Config.wifi_on));
          return 0;
        }
        Config.wifi_on = atob(args[1]);
        require_reboot = true;
      } else if(!strcmp(option, "bt_on")) {
        if (!args[1]) {
          Serial.println(String(Config.bt_on));
          return 0;
        }
        Config.bt_on = atob(args[1]);
        require_reboot = true;
      } else if(!strcmp(option, "led_brightness")) {
        if (!args[1]) {
          Serial.println(String(Config.led_brightness));
          return 0;
        }
        Config.led_brightness = atoi(args[1]);
      } else if(!strcmp(option, "websocket_port")) {
        if (!args[1]) {
          Serial.println(String(Config.websocket_port));
          return 0;
        }
        Config.websocket_port = atoi(args[1]);
        require_reboot = true;
      } else if(!strcmp(option, "motor_max_speed")) {
        if (!args[1]) {
          Serial.println(String(Config.motor_max_speed));
          return 0;
        }
        Config.motor_max_speed = atoi(args[1]);
      } else if(!strcmp(option, "screen_dim_seconds")) {
        if (!args[1]) {
          Serial.println(String(Config.screen_dim_seconds));
          return 0;
        }
        Config.screen_dim_seconds = atoi(args[1]);
      } else if(!strcmp(option, "pressure_smoothing")) {
        if (!args[1]) {
          Serial.println(String(Config.pressure_smoothing));
          return 0;
        }
        Config.pressure_smoothing = atoi(args[1]);
      } else if(!strcmp(option, "classic_serial")) {
        if (!args[1]) {
          Serial.println(String(Config.classic_serial));
          return 0;
        }
        Config.classic_serial = atob(args[1]);
      } else if(!strcmp(option, "sensitivity_threshold")) {
        if (!args[1]) {
          Serial.println(String(Config.sensitivity_threshold));
          return 0;
        }
        Config.sensitivity_threshold = atoi(args[1]);
      } else if(!strcmp(option, "motor_ramp_time_s")) {
        if (!args[1]) {
          Serial.println(String(Config.motor_ramp_time_s));
          return 0;
        }
        Config.motor_ramp_time_s = atoi(args[1]);
      } else if(!strcmp(option, "update_frequency_hz")) {
        if (!args[1]) {
          Serial.println(String(Config.update_frequency_hz));
          return 0;
        }
        Config.update_frequency_hz = atoi(args[1]);
      } else if(!strcmp(option, "sensor_sensitivity")) {
        if (!args[1]) {
          Serial.println(String(Config.sensor_sensitivity));
          return 0;
        }
        Config.sensor_sensitivity = atoi(args[1]);
        Hardware::setPressureSensitivity(Config.sensor_sensitivity);
      } else if (!strcmp(option, "knob_rgb")) {
        if (!args[1] || !args[2] || !args[3]) {
          Serial.println("Usage: set knob_rgb r g b");
          return 1;
        }
        Hardware::setEncoderColor(CRGB(atoi(args[1]), atoi(args[2]), atoi(args[3])));
      } else {
        Serial.println("Unknown config key: " + String(option));
        return 1;
      }

      saveConfigToSd(0);

      if (require_reboot) {
        Serial.println("A device reset will be required for the new settings to "
                       "take effect.");
      }

//      int p = 0;
//      char *c = args[p+1];
//      while (c != NULL) {
//        Serial.print(String(c) + " ");
//        p++;
//        c = args[p];
//      }
//      Serial.println();
    }

    int sh_bool(char **args) {
      int p = 0;
      char *c = args[p];
      while (c != NULL) {
        Serial.print(String(c) + "\t");
        Serial.println(atob(c) ? "TRUE" : "FALSE");
        p++;
        c = args[p];
      }
    }

    /**
     * Handles the message currently in the buffer.
     */
    void handleMessage() {
      const int TOK_BUFSIZE = 64;
      const char* TOK_DELIM = " \t\r\n\a";

      Serial.println("> " + String(buffer));

      int bufsize = TOK_BUFSIZE;
      int position = 0;

      char *tokens[bufsize] = { 0 };
      char *token;

      if (!tokens) {
        Serial.println("allocation error :(");
        return;
      }

      token = strtok(buffer, TOK_DELIM);
      while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position > bufsize) {
          // buffer overflow, extend?
        }

        token = strtok(NULL, TOK_DELIM);
      }

      tokens[position] = 0;

      // Empty command!
      if (position == 0) {
        return;
      }

      // Find Cmd
      char *cmd = tokens[0];
      bool handled = false;

      for (int i = 0; cmd[i]; i++) {
        // Command to lower
        cmd[i] = tolower(cmd[i]);
      }

      for (int i = 0; i < sizeof(commands)/sizeof(Command); i++) {
        Command c = commands[i];

        if (strcmp(c.cmd, cmd) == 0 || strcmp(c.alias, cmd) == 0) {
          c.func(tokens+1);
          handled = true;
          break;
        }
      }

      if (!handled) {
        Serial.println("Unknown command.");
      }

      buffer_i = 0;
      buffer[buffer_i] = 0;
    }
  }

  /**
   * Reads and stores incoming bytes onto the buffer until
   * we have a newline.
   */
  void loop() {
    while (Serial.available() > 0) {
      // read the incoming byte:
      char incoming = Serial.read();

      if (incoming == '\n') {
        handleMessage();
      } else {
        buffer[buffer_i] = incoming;
        buffer_i++;
        buffer[buffer_i] = 0;
      }
    }
  }
}