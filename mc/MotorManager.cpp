#include "MotorManager.h"

MotorManager::MotorManager() {
  Serial.println(">>>>>>>> MotorManager() >>>>>>>>");
}

void MotorManager::initMotor() {
  Serial.println(">>>>>>>> initMotor() >>>>>>>>");

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(MS0, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  
  this->stepper = AccelStepper(MotorInterfaceType, stepPin, dirPin);
  this->stepper.setMaxSpeed(1000);

  this->setStepResolution(StepType::_1_div_8);
}

void MotorManager::setStepResolution(StepType stepType) {
  /*
  M0	  M1	  M2	  Microstep Resolution
  Low	  Low	  Low	  Full step
  High	Low	  Low	  Half step
  Low	  High	Low	  1/4 step
  High	High	Low	  1/8 step
  Low	  Low	  High	1/16 step
  High	Low	  High	1/32 step
  Low	  High	High	1/32 step
  High	High	High	1/32 step
  */
  switch(stepType) {
    case StepType::Full:
      this->stepType = StepType::Full;
      digitalWrite(MS0, LOW);
      digitalWrite(MS1, LOW);
      digitalWrite(MS2, LOW);
      break;
    case StepType::Half:
      this->stepType = StepType::Half;
      digitalWrite(MS0, HIGH);
      digitalWrite(MS1, LOW);
      digitalWrite(MS2, LOW);
      break;
    case StepType::_1_div_4:
      this->stepType = StepType::_1_div_4;
      digitalWrite(MS0, LOW);
      digitalWrite(MS1, HIGH);
      digitalWrite(MS2, LOW);
      break;
    case StepType::_1_div_8:
      this->stepType = StepType::_1_div_8;
      digitalWrite(MS0, HIGH);
      digitalWrite(MS1, HIGH);
      digitalWrite(MS2, LOW);
      break;
    case StepType::_1_div_16:
      this->stepType = StepType::_1_div_16;
      digitalWrite(MS0, LOW);
      digitalWrite(MS1, LOW);
      digitalWrite(MS2, HIGH);
      break;
    case StepType::_1_div_32:
      this->stepType = StepType::_1_div_32;
      digitalWrite(MS0, HIGH);
      digitalWrite(MS1, LOW);
      digitalWrite(MS2, HIGH);
      break;
  }
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
  const TickType_t xDelay = 1000;
  for (;;)
  {
    switch(this->currentState) {
      case States::RUN:
        // Set the speed of the motor in steps per second:
        this->stepper.setSpeed(512);
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
