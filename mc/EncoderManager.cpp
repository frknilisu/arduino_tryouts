#include "EncoderManager.h"

#include <Wire.h>

int segmentCounter = 0;
int previousRawSegment = 0;
int currentRawSegment = 0;

EncoderManager::EncoderManager() {
  Serial.println(">>>>>>>> EncoderManager::EncoderManager() >>>>>>>>");
}

void EncoderManager::initEncoder() {
  Serial.println(">>>>>>>> EncoderManager::initEncoder() >>>>>>>>");
  
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
  return currentRawSegment;
}

/*
void checkSegment() {
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

Pair<DIRECTION, int> pointDiffOnSameSegment(const Point& p1, const Point& p2) {
  int rawRemainderDiff = p2.getRemainder() - p1.getRemainder();
  DIRECTION dir = rawRemainderDiff >= 0 ? DIRECTION::CW : DIRECTION::CCW;
  return Pair<DIRECTION, int>(dir, rawRemainderDiff);
}

int pair2StepDiff(Pair<DIRECTION, int> tempPointDiff) {
  return int(tempPointDiff.first()) * tempPointDiff.second();
}

Pair<DIRECTION, int> step2PairDiff(int encoderStepDiff) {
  return Pair<DIRECTION, int>(encoderStepDiff >= 0 ? DIRECTION::CW : DIRECTION::CCW, abs(encoderStepDiff));
}

Pair<DIRECTION, int> pointDiffOnDifferentSegment(const Point& p1, const Point& p2) {
  if(p1.getSegment() == p2.getSegment())
    return pointDiffOnSameSegment(p1, p2);
  
  int rawSegmentDiff = p2.getSegment() - p1.getSegment();
  int encoderStepDiff = 2^ENCODER_RESOLUTION * rawSegmentDiff;
  Pair<DIRECTION, int> tempPointDiff = pointDiffOnSameSegment(Point(p2.getSegment(), p1.getRemainder()), p2);
  int tempStepDiff = pair2StepDiff(tempPointDiff); 
  encoderStepDiff += tempStepDiff;
  return step2PairDiff(encoderStepDiff);
}

int convertEncoderToMotorStep (int encoderStep) {
  return round(encoderStep * (2^ENCODER_RESOLUTION / (200 / MOTOR_RESOLUTION)));
}
*/

void EncoderManager::runLoop() {
  this->initEncoder();
  
  //const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
  const TickType_t xDelay = 100;
  for (;;)
  {
    currentRawSegment = this->ams5600.getRawAngle();
    //checkSegment();
    previousRawSegment = currentRawSegment;
    vTaskDelay(xDelay);
  }
}
