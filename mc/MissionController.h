#ifndef MISSION_CONTROLLER_H
#define MISSION_CONTROLLER_H

//#include "BleManager.h"
#include "EncoderManager.h"
#include "MotorManager.h"
#include "ActionPoint.h"

class BleManager;

class MissionController {
  public:
    MissionController(EncoderManager* encoderManager, MotorManager* motorManager, BleManager* bleManager);
    void runLoop();

    void setStartProgramming(bool start);
    void setFinishProgramming(bool finish);
    void setA(int currentRawSegment, int segmentCounter);
    void setB(int currentRawSegment, int segmentCounter);
  private:
    enum class States {
      MANUAL,
      PROGRAMMING,
      ACTION,
      ERRORw
    };
    
    States currentState = States::MANUAL;

    ActionPoint pA, pB;
    bool isStartProgramming = false;
    bool isFinishProgramming = false;
    bool isSetA = false;
    bool isSetB = false;
    bool actionCompleted = false;

    BleManager* bleManager;
    EncoderManager* encoderManager;
    MotorManager* motorManager;
};

#endif
