#include "../../include/UIMenu.h"
#include "../../include/Page.h"

UIMenu GamesMenu("Games", [](UIMenu *menu) {
  menu->addItem("Snake", [](UIMenu*) {
    Page::Go(&SnakePage);
  });
});