#ifndef __UPDATEHELPER_H
#define __UPDATEHELPER_H

#include <Update.h>
#include <FS.h>
#include <SD_MMC.h>

enum UpdateSource {
  NoUpdate,
  UpdateFromSD,
  UpdateFromServer
};

namespace UpdateHelper {
  // perform the actual update from a given stream
  void performUpdate(Stream &updateSource, size_t updateSize);

  // check given FS for valid update.bin and perform update if available
  void updateFromFS(fs::FS &fs);

  // Update Available
  UpdateSource checkForUpdates();
}

#endif
