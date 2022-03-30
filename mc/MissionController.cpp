#include "MissionController.h"
#include "BleManager.h"
#include "Calculations.h"

using namespace Calculations;

MissionController::MissionController(EncoderManager* encoderManager, MotorManager* motorManager, BleManager* bleManager)
  : encoderManager(encoderManager), motorManager(motorManager), bleManager(bleManager) {
  Serial.println(">>>>>>>> MissionController() >>>>>>>>");
}

void MissionController::setA(int currentRawSegment, int segmentCounter) {
  this->pA.setRemainder(currentRawSegment);
  this->pA.setSegment(segmentCounter);
      
  //logCout = "Point A - Current Segment: " + String(currentRawSegment) + "\n" + \
  //           "Point A - Segment Counter: " + String(segmentCounter);
  
  this->isSetA = true;
}

void MissionController::setB(int currentRawSegment, int segmentCounter) {
  this->pB.setRemainder(currentRawSegment);
  this->pB.setSegment(segmentCounter);
      
  //logCout = "Point B - Current Segment: " + String(currentRawSegment) + "\n" + \
  //           "Point B - Segment Counter: " + String(segmentCounter);
  
  this->isSetB = true;
}

void MissionController::setStartProgramming(bool start) {
  this->isStartProgramming = start;
}

void MissionController::setFinishProgramming(bool finish) {
  this->isFinishProgramming = finish;
}

void MissionController::runLoop() {
  for(;;)
  {
    switch(this->currentState) {
      case States::MANUAL:
        if(bleManager->isDeviceConnected()) {
          //this->motorManager->manualControlEnable(true);
          if(this->isStartProgramming) {
            this->currentState = States::PROGRAMMING;
          }
        }
        break;
      case States::PROGRAMMING:
        //this->motorManager->manualControlEnable(true);
        if(this->isSetA && this->isSetB && this->isFinishProgramming) {
          this->currentState = States::ACTION;
          //this->motorManager->manualControlEnable(false);
        }
        break;
      case States::ACTION:
        if(encoderManager->isTargetReached() && this->actionCompleted) {
          this->currentState = States::PROGRAMMING;
        } else {
          int ss = convertEncoderStepToMotorStep(200);
          Pair<DIRECTION, int> pdi = pointDiffOnSameSegment(this->pA, this->pB);
          // int step = calculate(this->pA, this->pB);
          this->motorManager->move(200);
        }
        break;
      case States::ERRORw:
        Serial.println("Error Occured");
        break;
    }
  }
}
