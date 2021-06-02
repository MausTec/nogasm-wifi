#include "../../include/UIMenu.h"
#include "../../include/UIInput.h"
#include "../../include/Hardware.h"
#include "../../include/OrgasmControl.h"

UIInput MotorMaxSpeedInput("Motor Max Speed", [](UIMenu *ip) {
  UIInput *input = (UIInput*) ip;
  input->setMax(255);
  input->setValue(Config.motor_max_speed);
  input->setStep(16);

  input->onOpen([](UIMenu*) {
    OrgasmControl::pauseControl();
  });

  input->onClose([](UIMenu*) {
    OrgasmControl::resumeControl();
  });

  input->onChange([](int value) {
    Config.motor_max_speed = value;
    Hardware::setMotorSpeed(value);
  });

  input->onConfirm([](int value) {
    Config.motor_max_speed = value;
    saveConfigToSd(0);
  });
});

UIInput MotorStartSpeedInput("Motor Start Speed", [](UIMenu *ip) {
  UIInput *input = (UIInput*) ip;
  input->setMax(150);
  input->setStep(1);
  input->setValue(Config.motor_start_speed);
  input->onChange([](int value) {
    Config.motor_start_speed = value;
  });
  input->onConfirm([](int) {
    saveConfigToSd(0);
  });
});

UIInput MotorRampTimeInput("Motor Ramp Time", [](UIMenu *ip) {
  UIInput *input = (UIInput*) ip;
  input->setMax(120);
  input->setStep(5);
  input->setValue(Config.motor_ramp_time_s);
  input->onChange([](int value) {
    Config.motor_ramp_time_s = value;
  });
  input->onConfirm([](int) {
    saveConfigToSd(0);
  });
});

UIInput EdgeDelayInput("Edge Delay", [](UIMenu *ip) {
  UIInput *input = (UIInput*) ip;
  input->setMax(60);
  input->setStep(1);
  input->setValue(Config.edge_delay / 1000);
  input->onChange([](int value) {
    Config.edge_delay = value * 1000;
  });
  input->onConfirm([](int) {
    saveConfigToSd(0);
  });
});

UIInput MaxAdditionalDelayInput("Maximum Additional Delay", [](UIMenu *ip) {
  UIInput *input = (UIInput*) ip;
  input->setMax(60);
  input->setStep(1);
  input->setValue(Config.max_additional_delay / 1000);
  input->onChange([](int value) {
    Config.max_additional_delay = value * 1000;
  });
  input->onConfirm([](int) {
    saveConfigToSd(0);
  });
});

UIInput MinimumOnTimeInput("Minimum On Time", [](UIMenu *ip) {
  UIInput *input = (UIInput*) ip;
  input->setMax(5000);
  input->setStep(100);
  input->setValue(Config.minimum_on_time);
  input->onChange([](int value) {
    Config.minimum_on_time = value;
  });
  input->onConfirm([](int) {
    saveConfigToSd(0);
  });
});

UIInput ArousalLimitInput("Arousal Limit", [](UIMenu *ip) {
  UIInput *input = (UIInput*) ip;
  input->setMax(1023);
  input->setStep(16);
  input->setValue(Config.sensitivity_threshold);
  input->onChange([](int value) {
    Config.sensitivity_threshold = value;
  });
  input->onConfirm([](int) {
    saveConfigToSd(0);
  });
});

UIInput SensorSensitivityInput("Sensor Sensitivity", [](UIMenu *ip) {
  UIInput *input = (UIInput*) ip;
  input->setMax(255);
  input->setStep(1);
  input->setValue(Config.sensor_sensitivity);
  input->setPollPeriod(1000 / 30);
  input->getSecondaryValue([](int value) {
    return Hardware::getPressure();
  });
  input->onChange([](int value) {
    Config.sensor_sensitivity = value;
    Hardware::setPressureSensitivity(value);
  });
  input->onConfirm([](int) {
    saveConfigToSd(0);
  });
});

static void buildMenu(UIMenu *menu) {
  menu->addItem(&MotorMaxSpeedInput);
  menu->addItem(&MotorStartSpeedInput);
  menu->addItem(&MotorRampTimeInput);
  menu->addItem(&EdgeDelayInput);
  menu->addItem(&MaxAdditionalDelayInput);
  menu->addItem(&MinimumOnTimeInput);
  menu->addItem(&ArousalLimitInput);
  menu->addItem(&SensorSensitivityInput);
}

UIMenu EdgingSettingsMenu("Edging Settings", &buildMenu);