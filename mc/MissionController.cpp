#include "MissionController.h"

MissionController::MissionController(BleManager* bleManager, EncoderManager* encoderManager, MotorManager* motorManager)
  : bleManager(bleManager), encoderManager(encoderManager), motorManager(motorManager) {
  Serial.println(">>>>>>>> MissionController() >>>>>>>>");
}

MissionController::setA(int currentRawSegment, int segmentCounter) {
  this->pA.setRemainder(currentRawSegment);
  this->pA.setSegment(segmentCounter);
      
  logCout = "Point A - Current Segment: " + String(currentRawSegment) + "\n" + \
             "Point A - Segment Counter: " + String(segmentCounter);
  
  this->isSetA = true;
}

MissionController::setB(int currentRawSegment, int segmentCounter) {
  this->pB.setRemainder(currentRawSegment);
  this->pB.setSegment(segmentCounter);
      
  logCout = "Point B - Current Segment: " + String(currentRawSegment) + "\n" + \
             "Point B - Segment Counter: " + String(segmentCounter);
  
  this->isSetB = true;
}

MissionController::setStartProgramming(bool start) {
  this->isStartProgramming = start;
}

MissionController::setFinishProgramming(bool finish) {
  this->isFinishProgramming = finish;
}

void MissionController::runLoop() {
  for(;;)
  {
    switch(this->currentState) {
      case States::MANUAL:
        if(bleManager->isDeviceConnected() && this->isStartProgramming) {
          this->currentState = States::PROGRAMMING;
        }
        break;
      case States::PROGRAMMING:
        if(this->isSetA && this->isSetB && this->isFinishProgramming) {
          this->currentState = States::ACTION;
        }
        break;
      case States::ACTION:
        if(encoderManager->isTargetReached() && actionCompleted) {
          this->currentState = States::PROGRAMMING;
        } else {
          this->motorManager->move(this->pA, this->pB);
        }
        break;
      case States::ERROR:
        Serial.println("Error Occured");
        break;
    }
  }
}
