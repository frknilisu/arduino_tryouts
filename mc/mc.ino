#include <Wire.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <AccelStepper.h>

#include "AS5600.h"
#include "Point.h"
#include "Pair.h"

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

// Define the AccelStepper interface type
#define MotorInterfaceType AccelStepper::DRIVER
#define dirPin 2
#define stepPin 3

#define ENCODER_RESOLUTION  12
#define MOTOR_RESOLUTION    1/8

#define SERVICE_UUID_ENCODER        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_ENCODER "33d14ca1-1ba0-4247-bb85-0ea4504eb03d"

#define SERVICE_UUID_UART      "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

AccelStepper stepper;

AMS_5600 ams5600;
Point pA, pB;

BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic = NULL;
BLECharacteristic* pEncoderCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

String lastCout;
int segmentCounter = 0;
int homeSegment = 0;
int previousRawSegment = 0;
int currentRawSegment = 0;
int ang, lang = 0;
volatile bool joyPressed = false;
bool startProgramming = false;
bool setA = false;
bool setB = false;
volatile bool readIt = false;

enum class DIRECTION {
  CW = 1,
  CCW = -1
};

enum class BLEMsgsEnum {
  msg_StartProgramming,
  msg_FinishProgramming,
  msg_SetA,
  msg_SetB
};

BLEMsgsEnum hashit(std::string const& inString) {
  if (inString == "startProgramming") return BLEMsgsEnum::msg_StartProgramming;
  if (inString == "finishProgramming") return BLEMsgsEnum::msg_FinishProgramming;
  if (inString == "setA") return BLEMsgsEnum::msg_SetA;
  if (inString == "setB") return BLEMsgsEnum::msg_SetB;
}

/*------------------------------------------------------*/
/*---------------------- Callbacks ---------------------*/
/*------------------------------------------------------*/
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("***onConnect***");
    deviceConnected = true;
  }
  
  void onDisconnect(BLEServer* pServer) {
    Serial.println("***onDisconnect***");
    deviceConnected = false;
  }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);
        Serial.println();
        Serial.println("*********");
        
        switch(hashit(rxValue)) {
          case BLEMsgsEnum::msg_StartProgramming:
            Serial.println("-- startProgramming --");
            startProgramming = true;
            break;
          case BLEMsgsEnum::msg_FinishProgramming:
            Serial.println("-- finishProgramming --");
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
          case BLEMsgsEnum::msg_SetB:
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
  Serial.println(">>>>>>>> Init BLE >>>>>>>>");
  
  BLEDevice::init("MyESP32 BLE");
  
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE UART Service
  BLEService* pService = pServer->createService(SERVICE_UUID_UART);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  
  pRxCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void InitEncoder() {
  Serial.println(">>>>>>>> Init Encoder >>>>>>>>");
  
  Wire.begin(SDA, SCL);
  while(ams5600.detectMagnet() == 0) {
    delay(1000);
  }
  if(ams5600.detectMagnet() == 1 ) {
    Serial.print("Current Magnitude: ");
    Serial.println(ams5600.getMagnitude());
  } else {
    Serial.println("Can not detect magnet");
  }
}

void InitMotor() {
  Serial.println(">>>>>>>> Init Motor >>>>>>>>");
  
  stepper = AccelStepper(MotorInterfaceType, stepPin, dirPin);
  stepper.setMaxSpeed(1000);
}

void notifyEncoder() {
  pTxCharacteristic->setValue(currentRawSegment);
  pTxCharacteristic->notify();
}

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

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/
void TaskBLE(void *pvParameters) {
  (void) pvParameters;

  Serial.println("Starting ESP32 BLE Client application...");
  
  InitBLE();

  for (;;) // A Task shall never return or exit.
  {
    if (deviceConnected) {
      notifyEncoder();
      vTaskDelay(10); // bluetooth stack will go into congestion, if too many packets are sent
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
      vTaskDelay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
    }
    //vTaskDelay(1000); // Delay a second between loops.
  }
}

void TaskReadEncoder(void *pvParameters) {
  (void) pvParameters;

  Serial.println("Starting Encoder Reading Task...");

  InitEncoder();
  
  //const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
  const TickType_t xDelay = 100;
  for (;;)
  {
    currentRawSegment = ams5600.getRawAngle();
    checkSegment();
    previousRawSegment = currentRawSegment;
    vTaskDelay(xDelay);
  }
}

void TaskDriveMotor(void *pvParameters) {
  (void) pvParameters;

  Serial.println("Starting Motor Drıvıng Task...");

  InitMotor();
  
  //const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
  const TickType_t xDelay = 100;
  for (;;)
  {
    // Set the speed of the motor in steps per second:
    stepper.setSpeed(150);
    // Step the motor with constant speed as set by setSpeed():
    stepper.runSpeed();
    vTaskDelay(xDelay);
  }
}

/*--------------------------------------------------*/
/*---------------------- Main ----------------------*/
/*--------------------------------------------------*/
void setup() {
  Serial.begin(115200);
  
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    TaskBLE
    ,  "TaskBLE"
    ,  2048
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskReadEncoder
    ,  "TaskReadEncoder"
    ,  2048  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);


  xTaskCreatePinnedToCore(
    TaskDriveMotor
    ,  "TaskDriveMotor"
    ,  2048  // Stack size
    ,  NULL
    ,  3  // Priority
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
