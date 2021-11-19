#include <AccelStepper.h>

#define joyX A0
#define joyY A1
#define joyPressed

// Define the AccelStepper interface type
#define MotorInterfaceType AccelStepper::DRIVER

#define dirPin 2
#define stepPin 3

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper library with 28BYJ-48 stepper motor:
// AccelStepper stepper = AccelStepper(MotorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);
AccelStepper stepper = AccelStepper(MotorInterfaceType, stepPin, dirPin);

int xMap, yMap, xValue, yValue;

void setup() {
  Serial.begin(9600);
  
  // Set the maximum steps per second:
  stepper.setMaxSpeed(1000);
}
 
void loop() {
  // put your main code here, to run repeatedly:
  xValue = analogRead(joyX);
  yValue = analogRead(joyY);
  xMap = map(xValue, 0, 1023, 150, 750);
  yMap = map(yValue, 0, 1023, 150, 750);
 
  //print the values with to plot or view
  Serial.print(xValue);
  Serial.print(", ");
  Serial.print(yValue);
  Serial.print("\t");
  Serial.print(xMap);
  Serial.print(", ");
  Serial.print(yMap);
  Serial.println();

  if(xValue < 500) {
    // Set the speed of the motor in steps per second:
    stepper.setSpeed(-xMap);
    // Step the motor with constant speed as set by setSpeed():
    stepper.runSpeed();
  } else if(xValue > 600) {
    // Set the speed of the motor in steps per second:
    stepper.setSpeed(xMap);
    // Step the motor with constant speed as set by setSpeed():
    stepper.runSpeed();
  } else {
    stepper.stop();
  }
}
