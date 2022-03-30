#include <Wire.h>

#include "EncoderManager.h"

EncoderManager::EncoderManager() {
  Serial.println(">>>>>>>> EncoderManager() >>>>>>>>");
}

void EncoderManager::initEncoder() {
  Serial.println(">>>>>>>> initEncoder() >>>>>>>>");
  
  Wire.begin(SDA, SCL);
  while(!this->ams5600.detectMagnet()) {
    delay(1000);
  }
  if(this->ams5600.detectMagnet()) {
    Serial.print("Current Magnitude: ");
    Serial.println(this->ams5600.getMagnitude());
  } else {
    Serial.println("Can not detect magnet of Encoder");
  }
}

int EncoderManager::getCurrentRawSegment() {
  return this->currentRawSegment;
}

int EncoderManager::getSegmentCounter() {
  return this->segmentCounter;
}

bool EncoderManager::isTargetReached() {
  return true;
}

void EncoderManager::checkSegment() {
  Serial.println("-----checkSegment----");
  Serial.printf("Prev: %d, Curr: %d\n", previousRawSegment, currentRawSegment);
  if((previousRawSegment >= 3072 && previousRawSegment < 4096) && (currentRawSegment >= 0 && currentRawSegment < 1024)) {
    ++segmentCounter;
    Serial.println("segment counter is increased: " + String(segmentCounter));
  } else if((previousRawSegment >= 0 && previousRawSegment < 1024) && (currentRawSegment >= 3072 && currentRawSegment < 4096)) {
    --segmentCounter;
    Serial.println("segment counter is decreased: " + String(segmentCounter));
  }
}

void EncoderManager::runLoop() {
  this->initEncoder();
  
  //const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
  const TickType_t xDelay = 100;
  for (;;)
  {
    currentRawSegment = this->ams5600.getRawAngle();
    checkSegment();
    previousRawSegment = currentRawSegment;
    vTaskDelay(xDelay);
  }
}
