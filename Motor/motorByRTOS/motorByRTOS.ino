#include "MotorManager.h"
#include "BleManager.h"

MotorManager* motorManager;
BleManager* bleManager;

void TaskMotor(void *pvParameters) {
  Serial.println("Starting Motor Driving Task...");

  motorManager->runLoop();
}

void TaskBLE(void* pvParameters) {
  Serial.println("Starting BLE Task...");
  
  bleManager->runLoop();
}

void setup() {
  Serial.begin(115200);

  motorManager = new MotorManager();
  bleManager = new BleManager(motorManager);

  xTaskCreatePinnedToCore(
    TaskMotor
    ,  "TaskMotor"
    ,  10000
    ,  NULL
    ,  2
    ,  NULL
    ,  0);

  xTaskCreatePinnedToCore(
    TaskBLE
    ,  "TaskBLE"
    ,  10000
    ,  NULL
    ,  2
    ,  NULL
    ,  1);
}

void loop() {
  
}