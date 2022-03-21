#include "MissionController.h"

#include "BleManager.h"

Point pA, pB;
bool startProgramming = false;
bool finishProgramming = false;
bool setA = false;
bool setB = false;

String lastCout;
int segmentCounter = 0;
int homeSegment = 0;
int previousRawSegment = 0;
int currentRawSegment = 0;
int ang, lang = 0;
volatile bool joyPressed = false;

volatile bool readIt = false;

enum class DIRECTION {
  CW = 1,
  CCW = -1
};

MissionController::MissionController(const BleManager& bleManager)
  : bleManager(bleManager) {
  
}

void MissionController::runLoop() {
  for(;;)
  {
    switch(currentState) {
      case States::MANUAL:
        if(bleManager.isDeviceConnected() && startProgramming) {
          currentState = States::PROGRAMMING;
        }
        break;
      case States::PROGRAMMING:
        if(setA && setB && finishProgramming) {
          currentState = States::ACTION;
        }
        break;
      case States::ACTION:
        if(encoderManager.isTargetReached() && actionCompleted) {
          currentState = States::PROGRAMMING;
        }
        break;
      case States::ERROR:
        Serial.println("Error Occured");
        break;
    }
  }
}
