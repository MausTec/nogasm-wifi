#include "../../include/UIMenu.h"
#include "../../include/UserInterface.h"
#include "../../include/Page.h"
#include "../../include/UpdateHelper.h"

static void buildMenu(UIMenu *menu) {
  menu->addItem("Automatic Edging", &RunGraphPage);

  menu->addItem(&EdgingSettingsMenu);
  menu->addItem(&UISettingsMenu);
  menu->addItem(&NetworkMenu);
  menu->addItem(&GamesMenu);

  menu->addItem("Update", [](UIMenu*) {
    UpdateSource source = UpdateHelper::checkForUpdates();
    if (source == UpdateFromSD) {
      UI.toastNow("Updating...", 0, false);
      UpdateHelper::updateFromFS(SD);
    } else {
      UI.toastNow("No valid updates.", 3000);
    }
  });
}

UIMenu MainMenu("Main Menu", &buildMenu);
