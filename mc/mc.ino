#include "Init.h"
#include "BleManager.h"
#include "EncoderManager.h"
#include "MotorManager.h"
#include "MissionController.h"

BleManager* bleManager;
EncoderManager* encoderManager;
MotorManager* motorManager;
MissionController* missionController;

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBLE(void* pvParameters) {
  Serial.println("Starting BLE Task...");
  
  bleManager->runLoop();
}

void TaskEncoder(void* pvParameters) {
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

  missionController->runLoop();
}

/*--------------------------------------------------*/
/*---------------------- Main ----------------------*/
/*--------------------------------------------------*/
void setup() {
  Serial.begin(115200);

  encoderManager = new EncoderManager();
  motorManager = new MotorManager();
  bleManager = new BleManager(encoderManager, motorManager, missionController);
  missionController = new MissionController(encoderManager, motorManager, bleManager);

  xTaskCreatePinnedToCore(
    TaskBLE
    ,  "TaskBLE"
    ,  10000
    ,  NULL
    ,  2
    ,  NULL
    ,  1);

  xTaskCreatePinnedToCore(
    TaskEncoder
    ,  "TaskEncoder"
    ,  10000
    ,  NULL
    ,  2
    ,  NULL
    ,  0);

  xTaskCreatePinnedToCore(
    TaskMotor
    ,  "TaskMotor"
    ,  10000
    ,  NULL
    ,  2
    ,  NULL
    ,  1);

  xTaskCreatePinnedToCore(
    TaskMissionControl
    ,  "TaskMissionControl"
    ,  10000
    ,  NULL
    ,  2
    ,  NULL
    ,  1);
  
}

void loop() {
  
}
