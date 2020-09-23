#include "../include/Page.h"
#include "../include/UserInterface.h"

Page* Page::currentPage = nullptr;
Page* Page::previousPage = nullptr;
Page* Page::previousPages[HISTORY_LENGTH] = { nullptr };
size_t Page::historyIndex = 0;

void Page::Go(Page* page, bool saveHistory) {
  // Close Menus & toasts
  UI.openMenu(nullptr, false, false);
  UI.toast("");

  // Skip Current Page
  if (page == currentPage) {
    Reenter();
    return;
  }

  previousPage = currentPage;

  if (previousPage != nullptr)
    previousPage->Exit();

  if (saveHistory)
    pushHistory(page);

  currentPage = page;
  currentPage->Enter();
  Rerender();
}

void Page::GoBack() {
  Page* prev = popHistory();
  if (prev != nullptr) {
    Go(prev, false);
  }
}

void Page::Reenter() {
  if (currentPage != nullptr) {
    currentPage->Enter(false);
  }
}

void Page::Rerender() {
  // Skip rendering the page IFF a menu is open.
  // We could technically still render and the menu should write over it but,
  // that's wasted ticks, and Render() should not assume to ever be called on
  // a Page.
  if (UI.isMenuOpen()) {
    return;
  }

  UI.clear(false);

  if (currentPage != nullptr) {
    currentPage->Render();
  }

  UI.drawToast();
  UI.render();
}

void Page::DoLoop() {
  if (currentPage != nullptr)
    currentPage->Loop();
}

// Instance Methods:

void Page::Loop() {
  // noop
}

void Page::Enter(bool) {
  // noop
}

void Page::Exit() {
  // noop
}

void Page::Render() {
  // noop
}

void Page::onKeyPress(byte i) {
  // noop
}

void Page::onEncoderChange(int diff) {
  // noop
}

// Private Methods

void Page::pushHistory(Page *page) {
  historyIndex = (historyIndex + 1) % HISTORY_LENGTH;
  previousPages[historyIndex] = page;
}

Page* Page::popHistory() {
  historyIndex = (historyIndex - 1) % HISTORY_LENGTH;
  return previousPages[historyIndex];
}

Page* Page::CurrentPage() {
  return currentPage;
}

// Instantiate UI Pages
pDebug DebugPage = pDebug();
pRunGraph RunGraphPage = pRunGraph();
pSnake SnakePage = pSnake();