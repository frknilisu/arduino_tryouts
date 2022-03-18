// ConstantSpeed.pde
// -*- mode: C++ -*-
//
// Shows how to run AccelStepper in the simplest,
// fixed speed mode with no accelerations
/// \author  Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2009 Mike McCauley
// $Id: ConstantSpeed.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

#include <AccelStepper.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define dirPin 26
#define stepPin 25
#define MS0 27
#define MS1 16
#define MS2 17
#define enablePin 12
#define motorInterfaceType 1

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

/* Configure type of Steps on Easy Driver:
// MS1 MS2 
//
// LOW LOW   = Full Step //
// HIGH LOW  = Half Step //
// LOW HIGH  = A quarter of Step //
// HIGH HIGH = An eighth of Step //
*/ 


#define Slider_Pin A0    // Arduino A0 Pin connected to the analog out of the Slider
#define change_switch 14    // Tact switch used to select stepping type

int change_switch_state=0;  // used to debounce the switch

int current_stepping=0;  // Used to store the current stepping type (0-5)

int Slider_Value;        // Used to save the current Analog Slider Value
int Stepper_Speed;       // Used to set the travel speed of the stepper motor
int Stepper_Direction;   // Used to choose the direction of travel

void setup() {
  Serial.begin(115200);

  Stepper_Direction=stepper.currentPosition();  // Set the starting position of the stepper which is equal to zero at startup

  pinMode(change_switch, INPUT_PULLUP);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  //pinMode(enablePin, OUTPUT);
  
  // set stepping type to FULL Steps
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);

  stepper.setEnablePin(8);
  stepper.setPinsInverted(false, false, true);
  //stepper.enableOutputs();
}
 
void loop() 
{
    // Change Switch - Debounce using a delay() command
    if (digitalRead(change_switch) == LOW && change_switch_state == 0) {
      delay(100);  // The higher the Delay the less chance of bouncing      
      change_switch_state=1;
      current_stepping++;
      
      switch (current_stepping) {
        case 1:
          digitalWrite(MS1, HIGH);
          digitalWrite(MS2, LOW);
          //"Stepping = HALF"
          break;

        case 2:
          digitalWrite(MS1, LOW);
          digitalWrite(MS2, HIGH);
          //"Stepping = 1/4"
          break;    

        case 3:
          digitalWrite(MS1, HIGH);
          digitalWrite(MS2, HIGH);
          //"Stepping = 1/8"
          break; 

        case 4:
          digitalWrite(MS1, LOW);
          digitalWrite(MS2, LOW);
          current_stepping=0;
          //"Stepping = FULL"
          break;
      }

    } else {
      if (change_switch_state == 1 && digitalRead(change_switch) == HIGH) {
        change_switch_state=0;
      }
    }

    Slider_Value=analogRead(Slider_Pin);  // Read the value of the analog slider
    
    if (Slider_Value > 575) {  // If slider is moved to the Right
      
        Stepper_Speed=map(Slider_Value,575,1023,1,700);  // Map the Right value of the slider to a Speed value for the stepper
        Stepper_Direction--;  // Decrease the position to reach by one which will be used to move the stepper later
        stepper.enableOutputs();
      
    } else if (Slider_Value < 350) {  // If slider is moved to the Left
      
        Stepper_Speed=map(Slider_Value,350,0,1,700);  // // Map the Left value of the slider to a Speed value for the stepper
        Stepper_Direction++;   // Increase the position to reach by one which will be used to move the stepper later
        stepper.enableOutputs();
        
    } else {
        Stepper_Speed=0;  // If slider is around the middle then don't move the stepper by setting the speed to zero
        stepper.disableOutputs();
    }
  
    // Move the stepper to new position
    stepper.moveTo(Stepper_Direction); 
    stepper.setSpeed(Stepper_Speed);
    stepper.setMaxSpeed(1000);

    // Do this until the stepper as reached the new destination
    while (stepper.distanceToGo() != 0) {  // if stepper hasn't reached new position
      stepper.runSpeedToPosition();  // move the stepper until new position reached
    }
    Serial.println(current_stepping);
}
