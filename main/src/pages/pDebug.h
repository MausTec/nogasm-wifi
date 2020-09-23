#ifndef __pDEBUG_h
#define __pDEBUG_h

#include "../../include/Page.h"
#include "../../include/UserInterface.h"
#include "../../include/assets.h"
#include "../../../VERSION.h"

class pDebug : public Page {
  void Enter(bool) override {
    Serial.println("Debug page entered.");
  }

  void Render() override {
    UI.display->drawBitmap(0, 0, SPLASH_IMG, 128, 64, SSD1306_WHITE);
    UI.display->setCursor(0, SCREEN_HEIGHT - 8);
    UI.display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    UI.display->print(VERSION);
    UI.render();
  }
};

#endif