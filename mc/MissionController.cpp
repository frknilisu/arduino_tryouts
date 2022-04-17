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
  this->isSetA = true;
}

void MissionController::setB(int currentRawSegment, int segmentCounter) {
  this->pB.setRemainder(currentRawSegment);
  this->pB.setSegment(segmentCounter);
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
          Pair<DIRECTION, int> diff = calculatePointDifference(this->pA, this->pB);
          this->motorManager->move(diff.second());
        }
        break;
      case States::ERROR:
        Serial.println("Error Occured");
        break;
    }
  }
}
