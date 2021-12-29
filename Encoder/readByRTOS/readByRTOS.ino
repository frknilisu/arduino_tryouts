/* Hello World Example
 
   This example code is in the Public Domain (or CC0 licensed, at your option.)
 
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <Wire.h>
#include "AS5600.h"

#define SDA 21
#define SCL 22

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

void TaskEncoderRead( void *pvParameters );

AMS_5600 ams5600;

int ang, lang = 0;
volatile bool readIt = false;

float convertRawAngleToDegrees(word newAngle) {
  /* Raw data reports 0 - 4095 segments, which is 0.087 of a degree */
  float retVal = newAngle * 0.087;
  ang = retVal;
  return ang;
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  xTaskCreatePinnedToCore(
    TaskEncoderRead
    ,  "TaskEncoderRead"
    ,  1024  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskEncoderRead(void *pvParameters)
{
  (void) pvParameters;
  
  Serial.println("Read_encoder");
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
  const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
  for (;;)
  {
    Serial.println(String(convertRawAngleToDegrees(ams5600.getRawAngle()),DEC));      
    vTaskDelay(xDelay);
  }
}
