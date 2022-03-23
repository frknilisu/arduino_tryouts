#ifndef MISSION_CONTROLLER_H
#define MISSION_CONTROLLER_H

#include "BleManager.h"
#include "EncoderManager.h"
#include "MotorManager.h"
#include "ActionPoint.h"

class MissionController {
  public:
    MissionController(BleManager* bleManager, EncoderManager* encoderManager, MotorManager* motorManager);
    void runLoop();

    void setStartProgramming(bool);
    void setFinishProgramming(bool);
    void setA(int currentRawSegment, int segmentCounter);
    void setB(int currentRawSegment, int segmentCounter);
  private:
    enum class States {
      MANUAL,
      PROGRAMMING,
      ACTION,
      ERROR
    }

    States currentState;
    ActionPoint pA, pB;
    bool isStartProgramming = false;
    bool isFinishProgramming = false;
    bool isSetA = false;
    bool isSetB = false;

    BleManager* bleManager;
    EncoderManager* encoderManager;
    MotorManager* motorManager;
};

#endif
