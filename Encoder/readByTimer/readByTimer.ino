#include <Arduino.h>
#include <Wire.h>
#include "AS5600.h"

#define SDA 21
#define SCL 22

/* create a hardware timer */
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
AMS_5600 ams5600;

int ang, lang = 0;
volatile bool readIt = false;

float convertRawAngleToDegrees(word newAngle) {
  /* Raw data reports 0 - 4095 segments, which is 0.087 of a degree */
  float retVal = newAngle * 0.087;
  ang = retVal;
  return ang;
}

void IRAM_ATTR onTimer(){
  portENTER_CRITICAL_ISR(&timerMux);
  Serial.println("onTimer");
  //Serial.println(String(convertRawAngleToDegrees(ams5600.getRawAngle()),DEC));
  readIt = true;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);

  Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>> ");
  while(ams5600.detectMagnet() == 0) {
    delay(1000);
  }
  if(ams5600.detectMagnet() == 1 ){
      Serial.print("Current Magnitude: ");
      Serial.println(ams5600.getMagnitude());
  }
  else{
      Serial.println("Can not detect magnet");
  }

  /* Use 1st timer of 4 */
  /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
  timer = timerBegin(0, 800, true);

  /* Attach onTimer function to our timer */
  timerAttachInterrupt(timer, &onTimer, true);

  /* Set alarm to call onTimer function every second 1 tick is 1us
  => 1 second is 1000000us */
  /* Repeat the alarm (third parameter) */
  timerAlarmWrite(timer, 1000000, true);

  /* Start an alarm */
  timerAlarmEnable(timer);
  Serial.println("start timer");
}

void loop() {
  //Serial.println("Loop");
  //delay(1000);
  if(readIt) {
    portENTER_CRITICAL(&timerMux);
    Serial.println(String(convertRawAngleToDegrees(ams5600.getRawAngle()),DEC));
    readIt = false;
    portEXIT_CRITICAL(&timerMux);
  }
}
