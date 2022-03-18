#include <AccelStepper.h>
#include <MultiStepper.h>

// Motor pin definitions:
#define motorPin1  2      // IN1 on the ULN2003 driver
#define motorPin2  3      // IN2 on the ULN2003 driver
#define motorPin3  4      // IN3 on the ULN2003 driver
#define motorPin4  5      // IN4 on the ULN2003 driver

// Define the AccelStepper interface type
#define MotorInterfaceType AccelStepper::DRIVER
// #define MotorInterfaceType AccelStepper::HALF4WIRE
#define dirPin 26
#define stepPin 25

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper library with 28BYJ-48 stepper motor:
// AccelStepper stepper = AccelStepper(MotorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);
AccelStepper stepper = AccelStepper(MotorInterfaceType, stepPin, dirPin);

void setup() {
  // Set the maximum steps per second:
  stepper.setMaxSpeed(1000);
}

void loop() {
  // Set the speed of the motor in steps per second:
  stepper.setSpeed(150);
  // Step the motor with constant speed as set by setSpeed():
  stepper.runSpeed();
}
