#ifndef __UIMenu_h
#define __UIMenu_h

#include "../../config.h"
#include <functional>

#define TITLE_SIZE 16

/**
 * Time in ms to ignore scroll events.
 *
 * This prevents that absolutely infuriating thing my CR-10 does when I go to scroll one item and it
 * registers two clicks because I was *kinda sorta* between the next thing. However, this also makes
 * it super annoying to scroll through menus at light speed, so this has been reduced from 500ms to
 * 300ms. There is truly no winning, but having this debounce here seemed to make my life slightly
 * less full of fire and rage.
 *
 * If I was an actual dragon, I would have set the scroll wheel on fire. Do I need larger smoothing
 * capacitors? 100nF was too big, 10nF seemed just right, but I still have this absolutely annoying
 * bounce that sometimes happens when you're in-between detents on the scroll wheel. Is this an issue
 * with the PEL12T? The no-name (also a PEL12 variant) SparkFun encoder does this too. I think every
 * encoder in the universe does this (see CR-10).
 *
 * Maybe I just need to learn patience when scrolling.
 * That would explain why the scroll wheel on my mouse is wearing out.
 *
 * But look, that scroll wheel has seen the duality of heal staff and murder gun when I mained Mercy,
 * and that is a pretty tense environment when I'm furiously scrolling to grab my pistol and shoot
 * a Junkrat for--     what was I on about?
 *
 * Oh yeah, rotary encoders are of the devil.
 */
#define MENU_SCROLL_DEBOUNCE 50

class UIMenu;
class Page;

//typedef void(*MenuCallback)(UIMenu*);
typedef std::function<void(UIMenu*)> MenuCallback;

typedef struct UIMenuItem {
  char *text;
  MenuCallback cb;
  UIMenuItem *next = nullptr;
  UIMenuItem *prev = nullptr;
} UIMenuItem;

class UIMenu {
public:
  // Construction
  UIMenu(char *t, MenuCallback = nullptr);
  void initialize();

  // Item Manipulation
  void addItem(char *text, MenuCallback cb = nullptr);
  void addItem(char *text, Page *p);
  void addItem(UIMenu *submenu);
  void clearItems();

  // Render Lifecycle
  void open(UIMenu *previous = nullptr, bool save_history = true);
  void render();
  void tick();
  UIMenu *close();

  // Selection Handling
  void selectNext();
  void selectPrev();
  void handleClick();
  int getItemCount();
  int getCurrentPosition();

private:
  char title[TITLE_SIZE + 1];

  UIMenu *prev = nullptr;

  UIMenuItem *first_item = nullptr;
  UIMenuItem *last_item = nullptr;
  UIMenuItem *current_item = nullptr;

  MenuCallback initializer = nullptr;
  long last_menu_change = 0;
};

extern UIMenu MainMenu;
extern UIMenu GamesMenu;
extern UIMenu NetworkMenu;
extern UIMenu UISettingsMenu;
extern UIMenu EdgingSettingsMenu;

#endif