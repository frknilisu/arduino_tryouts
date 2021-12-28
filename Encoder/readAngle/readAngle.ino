#include <Wire.h>
#include "AS5600.h"

#define SDA 21
#define SCL 22

AMS_5600 ams5600;

int ang, lang = 0;

void setup()
{
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
}
/*******************************************************
/* Function: convertRawAngleToDegrees
/* In: angle data from AMS_5600::getRawAngle
/* Out: human readable degrees as float
/* Description: takes the raw angle and calculates
/* float value in degrees.
/*******************************************************/
float convertRawAngleToDegrees(word newAngle)
{
  /* Raw data reports 0 - 4095 segments, which is 0.087 of a degree */
  float retVal = newAngle * 0.087;
  ang = retVal;
  return ang;
}
void loop()
{
    Serial.println(String(convertRawAngleToDegrees(ams5600.getRawAngle()),DEC));
}
