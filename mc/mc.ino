#include <Wire.h>
#include "AS5600.h"
#include "Point.h"

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL Serial
  #define SYS_VOL   3.3
#else
  #define SERIAL Serial
  #define SYS_VOL   5
#endif

#define SDA 21
#define SCL 22

AMS_5600 ams5600;
Point pA, pB;

String lastResponse;
String noMagnetStr = "Error: magnet not detected";

int ang, lang = 0;
int segmentCounter = 0;
int homeSegment = 0;
int previousRawSegment = 0;
int currentRawSegment = 0;

void setup() {
  Wire.begin(SDA, SCL);
  SERIAL.begin(115200);
  SERIAL.println(">>>>>>>>>>>>>>>>>>>>>>>>>>> ");
  while(ams5600.detectMagnet() == 0) {
    delay(1000);
  }
  if(ams5600.detectMagnet() == 1 ){
      SERIAL.print("Current Magnitude: ");
      SERIAL.println(ams5600.getMagnitude());

      SERIAL.print("Current Raw Angle: ");
      homeSegment = ams5600.getRawAngle();
      SERIAL.println(String(convertRawAngleToDegrees(homeSegment), DEC));
  }
  else{
      SERIAL.println("Can not detect magnet");
  }
  printMenu();
}


/*******************************************************
/* function: printMenu
/* In: none
/* Out: none
/* Description: prints menu options and result of last
/* command
/*******************************************************/
void printMenu()
{
  for(int i=0; i<20;i++)
    SERIAL.println();
  SERIAL.println("AS5600 SERIAL Interface Program");
  SERIAL.println("");
  if(lastResponse.length()>0)
  {
    SERIAL.println(lastResponse);
    SERIAL.println("");
  }
  SERIAL.print("1 - Set start position\t|  "); SERIAL.println(" 6 - get end position");
  SERIAL.print("2 - Set end position\t|  ");   SERIAL.println(" 7 - get raw angle");
  SERIAL.print("3 - Set max angle range\t|  ");  SERIAL.println(" 8 - get scaled angle");
  SERIAL.print("4 - Get max angle range\t|  ");  SERIAL.println(" 9 - detect magnet");
  SERIAL.print("5 - Get start position \t\t|  ");     SERIAL.println("10 - get magnet strength");
  SERIAL.print("11 - Set PointA \t\t|  ");     SERIAL.println("12 - set PointB");
  SERIAL.println();
  SERIAL.print("Number of burns remaining: "); SERIAL.println(String(3 - ams5600.getBurnCount()));
  SERIAL.println("96 - Burn Angle");
  SERIAL.println("98 - Burn Settings (one time)");
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
  return retVal;
}

/*******************************************************
/* Function: convertScaledAngleToDegrees
/* In: angle data from AMS_5600::getScaledAngle
/* Out: human readable degrees as float
/* Description: takes the scaled angle and calculates
/* float value in degrees.
/*******************************************************/
float convertScaledAngleToDegrees(word newAngle)
{
  word startPos = ams5600.getStartPosition();
  word endPos = ams5600.getEndPosition();
  word maxAngle = ams5600.getMaxAngle();

  float multipler = 0;

  /* max angle and end position are mutually exclusive*/
  if(maxAngle >0)
  {
    if(startPos == 0)
      multipler = (maxAngle*0.0878)/4096;
    else  /*startPos is set to something*/
      multipler = ((maxAngle*0.0878)-(startPos * 0.0878))/4096;
  }
  else
  {
    if((startPos == 0) && (endPos == 0))
      multipler = 0.0878;
    else if ((startPos > 0 ) && (endPos == 0))
      multipler = ((360 * 0.0878) - (startPos * 0.0878)) / 4096;
    else if ((startPos == 0 ) && (endPos > 0))
      multipler = (endPos*0.0878) / 4096;
    else if ((startPos > 0 ) && (endPos > 0))
      multipler = ((endPos*0.0878)-(startPos * 0.0878))/ 4096;
  }
  return (newAngle * multipler);
}

/*******************************************************
/* Function: burnAngle
/* In: none
/* Out: human readable string of success or failure
/* Description: attempts to burn angle data to AMS5600
/*******************************************************/
String burnAngle()
{
  int burnResult = ams5600.burnAngle();
  String returnStr = "Brun angle error: ";

  switch (burnResult)
  {
    case 1:
      returnStr = "Brun angle success";
      break;
    case -1:
      returnStr += "no magnet detected";
      break;
    case -2:
      returnStr += "no more burns left";
      break;
    case -3:
      returnStr += "no positions set";
      break;
    default:
      returnStr += "unknown";
      break;
  }
  return returnStr;
}

/*******************************************************
/* Function: burnMaxAngleAndConfig
/* In: none
/* Out: human readable string of sucess or failure
/* Description: attempts to burn max angle and config data
/* to AMS5600
/*******************************************************/
String burnMaxAngleAndConfig()
{
  int burnResult = ams5600.burnMaxAngleAndConfig();
  String retStr = "Brun max angle and config error: ";

  switch(burnResult)
  {
    case 1:
      retStr = "Brun max angle and config success";
      break;
    case -1:
      retStr += "chip has been burned once already";
      break;
    case -2:
      retStr += "max angle less than 18 degrees";
      break;
    default:
      retStr += "unknown";
      break;
  }
  return retStr;
}

void loop() {
  
  currentRawSegment = ams5600.getRawAngle();
  if((previousRawSegment >= 0 && previousRawSegment < 256) && (currentRawSegment >= 3840 && currentRawSegment < 4096)) {
    --segmentCounter;
  } else if((previousRawSegment >= 3840 && previousRawSegment < 4096) && (currentRawSegment >= 0 && currentRawSegment < 256)) {
    ++segmentCounter;
  }
  previousRawSegment = currentRawSegment;
  
  if (SERIAL.available() > 0)
  {
    char incomingByteBuffer[5] = {0};
    char incomingByte = 0;

    incomingByteBuffer[0] = NULL;
    incomingByteBuffer[1] = NULL;

    SERIAL.readBytes(incomingByteBuffer,2);

    if ((incomingByteBuffer[0] >= 48) && (incomingByteBuffer[0] < 60))
    {
      incomingByte = incomingByteBuffer[0] - 48;
    }

    if ((incomingByteBuffer[1] >= 48) && (incomingByteBuffer[1] < 60))
    {
      incomingByte *=10;
      incomingByte += incomingByteBuffer[1] - 48;
    }


    switch (incomingByte)
    {
      case 5:
      {
        lastResponse = ("Start angle = "+String(convertRawAngleToDegrees(ams5600.getStartPosition()), DEC));
      }
      break;

      case 6:
      {
        lastResponse = "End angle = " + String(convertRawAngleToDegrees(ams5600.getEndPosition()),DEC);
      }
      break;

      case 7:
      {
        lastResponse = "Raw angle = "+ String(convertRawAngleToDegrees(ams5600.getRawAngle()),DEC);
      }
      break;

      case 8:
      {
        lastResponse = "Scaled angle = "+String(convertScaledAngleToDegrees(ams5600.getScaledAngle()),DEC);
      }
      break;

      case 11:
      {
        currentRawSegment = ams5600.getRawAngle();
        pA.setRemainder(currentRawSegment);
        pA.setSegment(segmentCounter);

        lastResponse = "Point A - Current Segment: " + String(currentRawSegment) + "\n" + \
                        "Point A - Segment Counter: " + String(segmentCounter);
      }
      break;

      case 12:
      {
        currentRawSegment = ams5600.getRawAngle();
        pB.setRemainder(currentRawSegment);
        pB.setSegment(segmentCounter);

        lastResponse = "Point B - Current Segment: " + String(currentRawSegment) + "\n" + \
                        "Point B - Segment Counter: " + String(segmentCounter);
      }
      break;

      case 13:
      {
        lastResponse = "Current Segment: " + String(currentRawSegment) + "\n" + \
                        "Segment Counter: " + String(segmentCounter);
      }
      break;

      default:
      {
          lastResponse = "Invalid Entry";
      }
      break;
    }
    /*end of menu processing */
    printMenu();
  }
  delay(200);
}
