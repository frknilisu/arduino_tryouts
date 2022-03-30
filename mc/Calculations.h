#ifndef CALCULATIONS_H
#define CALCULATIONS_H

#define ENCODER_RESOLUTION  12
#define MOTOR_RESOLUTION    1/8

#include <Arduino.h>

#include "Pair.h"
#include "ActionPoint.h"

namespace Calculations
{
  enum class DIRECTION {
    CW = 1,
    CCW = -1
  };
  
  Pair<DIRECTION, int> pointDiffOnSameSegment(const ActionPoint& p1, const ActionPoint& p2) {
    int rawRemainderDiff = p2.getRemainder() - p1.getRemainder();
    DIRECTION dir = rawRemainderDiff >= 0 ? DIRECTION::CW : DIRECTION::CCW;
    return Pair<DIRECTION, int>(dir, rawRemainderDiff);
  }
  
  int pair2StepDiff(Pair<DIRECTION, int> tempPointDiff) {
    return int(tempPointDiff.first()) * tempPointDiff.second();
  }
  
  Pair<DIRECTION, int> step2PairDiff(int encoderStepDiff) {
    return Pair<DIRECTION, int>(encoderStepDiff >= 0 ? DIRECTION::CW : DIRECTION::CCW, abs(encoderStepDiff));
  }
  
  Pair<DIRECTION, int> pointDiffOnDifferentSegment(const ActionPoint& p1, const ActionPoint& p2) {
    if(p1.getSegment() == p2.getSegment())
      return pointDiffOnSameSegment(p1, p2);
    
    int rawSegmentDiff = p2.getSegment() - p1.getSegment();
    int encoderStepDiff = 2^ENCODER_RESOLUTION * rawSegmentDiff;
    Pair<DIRECTION, int> tempPointDiff = pointDiffOnSameSegment(ActionPoint(p2.getSegment(), p1.getRemainder()), p2);
    int tempStepDiff = pair2StepDiff(tempPointDiff); 
    encoderStepDiff += tempStepDiff;
    return step2PairDiff(encoderStepDiff);
  }
  
  int convertEncoderStepToMotorStep(int encoderStep) {
      return round(encoderStep * (2^ENCODER_RESOLUTION / (200 / MOTOR_RESOLUTION)));
  }
}

#endif
