#include "EncoderManager.h"

EncoderManager* encoderManager;

void TaskEncoder(void* pvParameters) {
  Serial.println("Starting Encoder Task...");

  encoderManager->runLoop();
}

void setup() {
  Serial.begin(115200);

  encoderManager = new EncoderManager();

  xTaskCreatePinnedToCore(
    TaskEncoder
    ,  "TaskEncoder"
    ,  4096
    ,  NULL
    ,  2
    ,  NULL
    ,  0);

}

void loop() {

}
