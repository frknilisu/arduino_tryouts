#ifndef MOTOR_MANAGER_H
#define MOTOR_MANAGER_H

#include <AccelStepper.h>

// Define the AccelStepper interface type
#define MotorInterfaceType AccelStepper::DRIVER
#define dirPin 26
#define stepPin 25
#define MS0 27
#define MS1 16
#define MS2 17
#define enablePin 12

class MotorManager {
  public:
    MotorManager();
    void runLoop();

    void setMotorStatus(std::string stateName);
    void move(int step);
  private:
    enum class States {
      RUN, 
      STOP
    };

    AccelStepper stepper;
    States currentState;

    void initMotor();
};

#endif
