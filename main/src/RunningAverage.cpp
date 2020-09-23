#include "../include/RunningAverage.h"

void RunningAverage::addValue(long value) {
  size_t ra_window = min(Config.pressure_smoothing, (byte)RA_BUFFER_SIZE);

  ra_sum = ra_sum - ra_readings[ra_index];
  ra_readings[ra_index] = value;
  ra_sum = ra_sum + value;
  ra_index = (ra_index + 1) % ra_window;
  ra_averaged = ra_sum / ra_window;
}

long RunningAverage::getAverage() {
  return ra_averaged;
}