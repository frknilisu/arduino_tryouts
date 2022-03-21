#ifndef INIT_H
#define INIT_H

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

#define ENCODER_RESOLUTION  12
#define MOTOR_RESOLUTION    1/8

#endif
