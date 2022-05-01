#include "Init.h"
#include "MissionController.h"
#include "ActionPoint.h"

ActionPoint pA, pB;
bool isStartProgramming = false;
bool isFinishProgramming = false;
bool isSetA = false;
bool isSetB = false;

BaseType_t xReturn;

MissionController::MissionController() {
  Serial.println(">>>>>>>> MissionController() >>>>>>>>");
}

void MissionController::setA() {
  uint32_t encoderData = receiveEncoderData();
  pA.setData(encoderData);
  pA.print();
  isSetA = true;
}

void MissionController::setB() {
  uint32_t encoderData = receiveEncoderData();
  pB.setData(encoderData);
  pB.print();
  isSetB = true;
}

void MissionController::setStartProgramming() {
  isStartProgramming = true;
}

void MissionController::setFinishProgramming() {
  isFinishProgramming = true;
}

uint32_t receiveEncoderData() {
  uint32_t encoderData;
  xQueueReceive(qEncoderTask, &encoderData, portMAX_DELAY);
  return encoderData;
}

void MissionController::runLoop() {
  
  uint32_t notificationValue;
  for(;;)
  {
    switch(this->currentState) {
      case States::MANUAL:
        Serial.println("--- States::MANUAL ---");
        xReturn = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
        isStartProgramming = true;
        this->currentState = States::PROGRAMMING;
        break;
      case States::PROGRAMMING:
        Serial.println("--- States::PROGRAMMING ---");
        xReturn = xTaskNotifyWait(0, 0, &notificationValue, portMAX_DELAY);
        Commands_t* cmd = (Commands_t*)(notificationValue);
        if(xReturn == pdTRUE && (*cmd == Commands_t::SET_A_CMD)) {
          this->setA();
        }
        xReturn = xTaskNotifyWait(0, 0, &notificationValue, portMAX_DELAY);
        cmd = (Commands_t*)(notificationValue);
        if(xReturn == pdTRUE && (*cmd == Commands_t::SET_B_CMD)) {
          this->setB();
        }
        xReturn = xTaskNotifyWait(0, 0, &notificationValue, portMAX_DELAY);
        if(xReturn == pdTRUE && (*cmd == Commands_t::FINISH_PROGRAMMING_CMD)) {
          this->setFinishProgramming();
        }
        if(isSetA && isSetB && isFinishProgramming) {
          Serial.println("Point A and B are set. Finishing action programming..");
          this->currentState = States::ACTION;
        }
        break;
      case States::ACTION:
        Serial.println("--- States::ACTION ---");
        MotorActionCommand_t motorActionCmd;
        motorActionCmd.cmd = Commands_t::MOTOR_START_ACTION_CMD;
        motorActionCmd.pointA = pA;
        motorActionCmd.pointB = pB;
        motorActionCmd.direction = 1;
        xTaskNotify(motorTaskHandle, motorActionCmd, eSetValueWithOverwrite);
        xReturn = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY); // finished
        this->currentState = States::PROGRAMMING;
        break;
      case States::ERROR:
        Serial.println("Error Occured");
        break;
    }
    vTaskDelay(1000);
  }
}
