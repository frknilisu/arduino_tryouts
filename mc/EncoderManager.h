#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include "AS5600.h"

class EncoderManager {
  public:
    EncoderManager();
    void runLoop();

    int getCurrentRawSegment();
  private:
    AMS_5600 ams5600;

    void initEncoder();
};

#endif
