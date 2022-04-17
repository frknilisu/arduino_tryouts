#include "MotorManager.h"

MotorManager::MotorManager() {
  Serial.println(">>>>>>>> MotorManager() >>>>>>>>");
}

void MotorManager::initMotor() {
  Serial.println(">>>>>>>> initMotor() >>>>>>>>");
  
  this->stepper = AccelStepper(MotorInterfaceType, stepPin, dirPin);
  this->stepper.setMaxSpeed(1000);
}

void MotorManager::setMotorStatus(std::string stateName) {
  if(stateName == "RUN") {
    this->currentState = States::RUN;
  } else if(stateName == "STOP") {
    this->currentState = States::STOP;
  }
}

void MotorManager::move(int step) {
  this->stepper.runSpeed();
}

void MotorManager::runLoop() {
  this->initMotor();
  
  //const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
  const TickType_t xDelay = 100;
  for (;;)
  {
    switch(this->currentState) {
      case States::RUN:
        // Set the speed of the motor in steps per second:
        this->stepper.setSpeed(150);
        // Step the motor with constant speed as set by setSpeed():
        this->stepper.runSpeed();
        break;
      case States::STOP:
        this->stepper.stop();
        break;
    }
    
    vTaskDelay(xDelay);
  }
}
