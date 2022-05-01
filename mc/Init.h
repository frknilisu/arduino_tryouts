#ifndef INIT_H
#define INIT_H

#define joyX 2
#define joyY 4
#define joySW 14

#define SEND_SUCCESSFUL_BIT             (0x01 << 0)
#define OPERATION_TIMED_OUT_BIT         (0x01 << 1)
#define NO_INTERNET_CONNECTION_BIT      (0x01 << 2)
#define CANNOT_LOCATE_CLOUD_SERVER_BIT  (0x01 << 3)

TaskHandle_t missionControlTaskHandle;
TaskHandle_t bleTaskHandle;
TaskHandle_t motorTaskHandle;
TaskHandle_t encoderTaskHandle;

QueueHandle_t qEncoderTask;

typedef struct EncoderData
{
  uint32_t absoluteStep;
  uint32_t roundCounter;
} EncoderData_t;

typedef enum Commands
{
  START_PROGRAMMING_CMD,
  FINISH_PROGRAMMING_CMD,
  SET_A_CMD,
  SET_B_CMD,
  MOTOR_RUN_CMD,
  MOTOR_STOP_CMD,
  MOTOR_START_ACTION_CMD
} Commands_t;

typedef struct MotorActionCommand
{
  Commands_t cmd;   /* The operation to perform (read or write). */
  EncoderData_t pointA;    /* Identifies the data being read or written. */
  EncoderData_t pointB; /* Only used when writing data to the cloud server. */
  uint8_t direction;   /* The handle of the task performing the operation. */
} MotorActionCommand_t;

#endif
