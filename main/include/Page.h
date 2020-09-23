#ifndef __PAGE_h
#define __PAGE_h

#include "Arduino.h"

#define HISTORY_LENGTH 5

class Page {
public:
  // Navigation
  static void Go(Page* page, bool saveHistory = true);
  static void GoBack();
  static void DoLoop();
  static void Rerender();
  static void Reenter();
  static void AttachButtonHandlers();
  static Page* CurrentPage();

  // Page Lifecycle
  virtual void Render();
  virtual void Loop();
  virtual void Enter(bool reinitialize = true);
  virtual void Exit();

  // Event Hooks
  virtual void onKeyPress(byte i);
  virtual void onEncoderChange(int diff);

  static Page* currentPage;

private:
  static Page* previousPage;
  static Page* previousPages[HISTORY_LENGTH];
  static size_t historyIndex;

  static void pushHistory(Page* page);
  static Page* popHistory();
};

#include "../../src/pages/pDebug.h"
extern pDebug DebugPage;

#include "../../src/pages/pRunGraph.h"
extern pRunGraph RunGraphPage;

#include "../../src/pages/pSnake.h"
extern pSnake SnakePage;

#endif