#ifndef PINS_H
#define PINS_H

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define SDA 21
#define SCL 22

#define joyX 2
#define joyY 4
#define joySW 14

// Define the AccelStepper interface type
#define MotorInterfaceType AccelStepper::DRIVER
#define dirPin 26
#define stepPin 25
#define MS0 27
#define MS1 16
#define MS2 17
#define enablePin 12

#define ENCODER_RESOLUTION  12
#define MOTOR_RESOLUTION    1/8

#define SERVICE_UUID_ENCODER        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_ENCODER "33d14ca1-1ba0-4247-bb85-0ea4504eb03d"

#define SERVICE_UUID_UART      "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#endif
