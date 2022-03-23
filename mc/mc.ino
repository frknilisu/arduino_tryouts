#include "Init.h"
#include "BleManager.h"
#include "EncoderManager.h"
#include "MotorManager.h"
#include "MissionController.h"

String logCout;

BleManager* bleManager;
EncoderManager* encoderManager;
MotorManager* motorManager;
MissionController* missionController;

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBLE(void *pvParameters) {
  (void) pvParameters;

  Serial.println("Starting BLE Task...");
  
  bleManager->runLoop();
}

void TaskEncoder(void *pvParameters) {
  (void) pvParameters;

  Serial.println("Starting Encoder Task...");

  encoderManager->runLoop();
}

void TaskMotor(void *pvParameters) {
  (void) pvParameters;

  Serial.println("Starting Motor Driving Task...");

  motorManager->runLoop();
}

void TaskMissionControl(void *pvParameters) {
  (void) pvParameters;

  Serial.println("Starting Mission Control Task...");

  missionController.runLoop();
}

/*--------------------------------------------------*/
/*---------------------- Main ----------------------*/
/*--------------------------------------------------*/
void setup() {
  Serial.begin(115200);

  encoderManager = new EncoderManager();
  motorManager = new MotorManager();
  bleManager = new BleManager(encoderManager, motorManager);
  missionController = new MissionController();

  xTaskCreatePinnedToCore(
    TaskBLE
    ,  "TaskBLE"
    ,  4096
    ,  NULL
    ,  1
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskEncoder
    ,  "TaskEncoder"
    ,  4096
    ,  NULL
    ,  2
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskMotor
    ,  "TaskMotor"
    ,  4096
    ,  NULL
    ,  3
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskMissionControl
    ,  "TaskMissionControl"
    ,  4096
    ,  NULL
    ,  4
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);
  
}

void loop() {
  if(logCout.length()>0) {
    Serial.println(logCout);
    Serial.println("");
    logCout = "";
  }
}
