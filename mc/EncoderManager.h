#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include "AS5600.h"
#include "Pair.h"
#include "ActionPoint.h"

#define ENCODER_RESOLUTION  12
#define MOTOR_RESOLUTION    1/8

#define SDA 21
#define SCL 22

class EncoderManager {
  public:
    EncoderManager();
    void runLoop();

    int getCurrentRawSegment();
    int getSegmentCounter();
    bool isTargetReached();
  private:
    enum class DIRECTION {
      CW = 1,
      CCW = -1
    };

    AMS_5600 ams5600;
    int previousRawSegment = 0;
    int currentRawSegment = 0;
    int segmentCounter = 0;
    
    void initEncoder();
    void checkSegment();
    /*
    Pair<DIRECTION, int> pointDiffOnSameSegment(const ActionPoint& p1, const ActionPoint& p2);
    int pair2StepDiff(Pair<DIRECTION, int> tempPointDiff);
    Pair<DIRECTION, int> step2PairDiff(int encoderStepDiff);
    Pair<DIRECTION, int> pointDiffOnDifferentSegment(const ActionPoint& p1, const ActionPoint& p2);
    */
    int convertEncoderToMotorStep(int encoderStep);
};

#endif
