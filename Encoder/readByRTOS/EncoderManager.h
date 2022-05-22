#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include "AS5600.h"

#define ENCODER_RESOLUTION  12

#define SDA 21
#define SCL 22

class EncoderManager {
  public:
    EncoderManager();
    void runLoop();

    int getCurrentRawSegment();
    int getSegmentCounter();
  private:
    AMS_5600 ams5600;
    int previousRawSegment = 0;
    int currentRawSegment = 0;
    int segmentCounter = 0;
    
    void initEncoder();
    void checkSegment();
};

#endif
