#ifndef __RunningAverage_h
#define __RunningAverage_h

#include "Arduino.h"
#include "../../config.h"

#define RA_BUFFER_SIZE 32

class RunningAverage {
public:
  void addValue(long value);
  long getAverage();

private:
  long ra_index = 0;
  long ra_value = 0;
  long ra_sum = 0;
  long ra_readings[RA_BUFFER_SIZE] = {0};
  long ra_averaged = 0;
};

#endif