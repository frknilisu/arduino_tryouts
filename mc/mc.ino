#include <Wire.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "AS5600.h"
#include "Point.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define SDA 21
#define SCL 22

#define joyX 2
#define joyY 4
#define joySW 14

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

AMS_5600 ams5600;
Point pA, pB;

String lastCout;

int segmentCounter = 0;
int homeSegment = 0;
int previousRawSegment = 0;
int currentRawSegment = 0;

volatile bool joyPressed = false;
bool startProgramming = false;
bool setA = false;
bool setB = false;

int ang, lang = 0;
volatile bool readIt = false;

enum BLEMsgsEnum {
    msg_StartProgramming,
    msg_StopProgramming,
    msg_SetA,
    msg_SetB
};

BLEMsgsEnum hashit(std::string const& inString) {
    if (inString == "startProgramming") return BLEMsgsEnum::msg_StartProgramming;
    if (inString == "stopProgramming") return BLEMsgsEnum::msg_StopProgramming;
    if (inString == "setA") return BLEMsgsEnum::msg_SetA;
    if (inString == "setB") return BLEMsgsEnum::msg_SetB;
}

/*------------------------------------------------------*/
/*---------------------- Callbacks ---------------------*/
/*------------------------------------------------------*/
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("***onConnect***");
    //_BLEClientConnected = true;
  }
  
  void onDisconnect(BLEServer* pServer) {
    Serial.println("***onDisconnect***");
    //_BLEClientConnected = false;
  }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        //Serial.print(value);
        Serial.println();
        Serial.println("*********");
        
        switch(hashit(value)) {
          case BLEMsgsEnum::msg_StartProgramming:
            Serial.println("-- startProgramming --");
            startProgramming = true;
            break;
          case BLEMsgsEnum::msg_StopProgramming:
            Serial.println("-- stopProgramming --");
            startProgramming = false;
            break;
          case BLEMsgsEnum::msg_SetA:
            Serial.println("-- setA --");
            
            currentRawSegment = ams5600.getRawAngle();
            pA.setRemainder(currentRawSegment);
            pA.setSegment(segmentCounter);
            
            lastCout = "Point A - Current Segment: " + String(currentRawSegment) + "\n" + \
                       "Point A - Segment Counter: " + String(segmentCounter);
            
            setA = true;
            break;
          case msg_SetB:
            Serial.println("-- setB --");
            
            currentRawSegment = ams5600.getRawAngle();
            pB.setRemainder(currentRawSegment);
            pB.setSegment(segmentCounter);
            
            lastCout = "Point B - Current Segment: " + String(currentRawSegment) + "\n" + \
                       "Point B - Segment Counter: " + String(segmentCounter);
            
            setA = true;
            break;
        }
      }
    }
};

/*--------------------------------------------------------------*/
/*---------------------- Utility Functions ---------------------*/
/*--------------------------------------------------------------*/
void InitBLE() {
  BLEDevice::init("MyESP32 BLE");
  
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create the BLE Characteristic
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  pCharacteristic->setValue("Hello World");

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  
  pAdvertising->addServiceUUID(pService->getUUID());
  
  pService->start();
  
  // Start advertising
  pAdvertising->start();
}

float convertRawAngleToDegrees(word newAngle) {
  /* Raw data reports 0 - 4095 segments, which is 0.087 of a degree */
  float retVal = newAngle * 0.087;
  ang = retVal;
  return ang;
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBLEAdvertise(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  Serial.println("Starting ESP32 BLE Client application...");
  
  InitBLE();

  for (;;) // A Task shall never return or exit.
  {
    //Serial.println("TaskBLEAdvertise - Loop Start ");
    
    //Serial.println("TaskBLEscanAndWrite - 1 Sec Task Delay");
    vTaskDelay(1000); // Delay a second between loops.
  }
}

void TaskReadEncoder(void *pvParameters)
{
  (void) pvParameters;
  
  Wire.begin(SDA, SCL);
  Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>> ");
  Serial.println("Read Encoder");
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
  //const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
  const TickType_t xDelay = 100;
  for (;;)
  {
    Serial.println(ams5600.getRawAngle());      
    vTaskDelay(xDelay);
  }
}

void setup() {
  Serial.begin(115200);
  
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    TaskBLEAdvertise
    ,  "TaskBLEAdvertise"   // A name just for humans
    ,  2048  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskReadEncoder
    ,  "TaskReadEncoder"
    ,  2048  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);
  
}

void loop() {
  if(lastCout.length()>0) {
    Serial.println(lastCout);
    Serial.println("");
    lastCout = "";
  }
}
