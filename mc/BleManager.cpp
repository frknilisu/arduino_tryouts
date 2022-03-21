#include "BleManager.h"

#include <BLEDevice.h>
#include <BLE2902.h>

bool deviceConnected = false;
bool oldDeviceConnected = false;
std::string lastReceivedMsg;

enum class BLEMsgsEnum {
  msg_StartProgramming,
  msg_FinishProgramming,
  msg_SetA,
  msg_SetB,
  msg_MotorRun,
  msg_MotorStop
};

BLEMsgsEnum hashit(std::string const& inString) {
  if (inString == "startProgramming") return BLEMsgsEnum::msg_StartProgramming;
  if (inString == "finishProgramming") return BLEMsgsEnum::msg_FinishProgramming;
  if (inString == "setA") return BLEMsgsEnum::msg_SetA;
  if (inString == "setB") return BLEMsgsEnum::msg_SetB;
  if (inString == "motorRun") return BLEMsgsEnum::msg_MotorRun;
  if (inString == "motorStop") return BLEMsgsEnum::msg_MotorStop;
}

/*--------------------------------------------------------------*/
/*---------------------- Utility Functions ---------------------*/
/*--------------------------------------------------------------*/

void BleManager::handleMsg(std::string receivedMsg) {
  lastReceivedMsg = "";
  switch(hashit(receivedMsg)) {
    case BLEMsgsEnum::msg_StartProgramming:
      Serial.println("-- Received Msg: startProgramming --");
      //missionController->startProgramming = true;
      break;
    case BLEMsgsEnum::msg_FinishProgramming:
      Serial.println("-- Received Msg: finishProgramming --");
      //missionController->startProgramming = false;
      break;
    case BLEMsgsEnum::msg_SetA:
      Serial.println("-- Received Msg: setA --");
      /*
      int currentRawSegment = encoderManager.getCurrentRawSegment();
      int segmentCounter = encoderManager.getSegmentCounter();
      missionController->setA(currentRawSegment, segmentCounter);
      pA.setRemainder(currentRawSegment);
      pA.setSegment(segmentCounter);
      
      lastCout = "Point A - Current Segment: " + String(currentRawSegment) + "\n" + \
                 "Point A - Segment Counter: " + String(segmentCounter);
      
      setA = true;
      */
      break;
    case BLEMsgsEnum::msg_SetB:
      Serial.println("-- Received Msg: setB --");
      /*
      int currentRawSegment = encoderManager.getCurrentRawSegment();
      int segmentCounter = encoderManager.getSegmentCounter();
      missionController->setB(currentRawSegment, segmentCounter);
      pB.setRemainder(currentRawSegment);
      pB.setSegment(segmentCounter);
      
      lastCout = "Point B - Current Segment: " + String(currentRawSegment) + "\n" + \
                 "Point B - Segment Counter: " + String(segmentCounter);
      
      setB = true;
      */
      break;
    case BLEMsgsEnum::msg_MotorRun:
      Serial.println("-- Received Msg: motorRun --");
      this->motorManager->setMotorStatus("RUN");
      break;
    case BLEMsgsEnum::msg_MotorStop:
      Serial.println("-- Received Msg: motorStop --");
      this->motorManager->setMotorStatus("STOP");
      break;
  }
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
    if (!rxValue.empty()) {
      lastReceivedMsg = rxValue;
    }
  }
};

BleManager::BleManager(EncoderManager* encoderManager, MotorManager* motorManager)
  : encoderManager(encoderManager), motorManager(motorManager) {
  Serial.println(">>>>>>>> BleManager() >>>>>>>>");
  this->currentState = States::INITIALIZING;
}

void BleManager::initBLE() {
  Serial.println(">>>>>>>> BleManager::initBLE() >>>>>>>>");
  
  BLEDevice::init("MyESP32 BLE");
  
  // Create the BLE Server
  this->pServer = BLEDevice::createServer();
  this->pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE UART Service
  this->pService = this->pServer->createService(SERVICE_UUID_UART);

  // Create a BLE Characteristic
  this->pTxCharacteristic = this->pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

  this->pTxCharacteristic->addDescriptor(new BLE2902());

  this->pRxCharacteristic = this->pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  
  this->pRxCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  
}

void BleManager::startAdvertising() {
  // Start the service
  this->pService->start();

  // Start advertising
  this->pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  this->currentState = States::LISTENING;
}

void BleManager::notifyEncoder() {
  int rss = encoderManager->getCurrentRawSegment();
  this->pTxCharacteristic->setValue(rss);
  this->pTxCharacteristic->notify();
}

void BleManager::runLoop() {
  for (;;) // A Task shall never return or exit.
  {
    switch(this->currentState) {
      case States::INITIALIZING:
        this->initBLE();
        this->currentState = States::START_ADVERTISING;
        break;
      case States::START_ADVERTISING:
        this->startAdvertising();
        break;
      case States::LISTENING:
        if (deviceConnected && !oldDeviceConnected) {
          // do stuff here on connecting
          oldDeviceConnected = deviceConnected;
          this->currentState = States::CONNECTED;
        }
        break;
      case States::CONNECTED:
        this->notifyEncoder();
        if(!lastReceivedMsg.empty()) {
          this->handleMsg(lastReceivedMsg);
        }
        vTaskDelay(10); // bluetooth stack will go into congestion, if too many packets are sent
      case States::DISCONNECTED:
        // disconnecting
        if (!deviceConnected && oldDeviceConnected) {
          vTaskDelay(500); // give the bluetooth stack the chance to get things ready
          oldDeviceConnected = deviceConnected;
          Serial.println("re-start advertising");
          this->currentState = States::START_ADVERTISING;
        }
        break;
    }
    //vTaskDelay(1000); // Delay a second between loops.
  }
}
