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
  switch(hashit(receivedMsg)) {
    case BLEMsgsEnum::msg_StartProgramming:
      Serial.println("-- Received Msg: startProgramming --");
      missionControlCommand.cmd = Commands_t::START_PROGRAMMING_CMD;
      xTaskNotify(missionControlTaskHandle, missionControlCommand, eSetValueWithOverwrite);
      break;
    case BLEMsgsEnum::msg_FinishProgramming:
      Serial.println("-- Received Msg: finishProgramming --");
      missionControlCommand.cmd = Commands_t::FINISH_PROGRAMMING_CMD;
      xTaskNotify(missionControlTaskHandle, missionControlCommand, eSetValueWithOverwrite);
      break;
    case BLEMsgsEnum::msg_SetA:
      Serial.println("-- Received Msg: setA --");
      missionControlCommand.cmd = Commands_t::SET_A_CMD;
      xTaskNotify(missionControlTaskHandle, missionControlCommand, eSetValueWithOverwrite);
      break;
    case BLEMsgsEnum::msg_SetB:
      Serial.println("-- Received Msg: setB --");
      missionControlCommand.cmd = Commands_t::SET_B_CMD;
      xTaskNotify(missionControlTaskHandle, missionControlCommand, eSetValueWithOverwrite);
      break;
    case BLEMsgsEnum::msg_MotorRun:
      Serial.println("-- Received Msg: motorRun --");
      motorActionCommand.cmd = Commands_t::MOTOR_RUN_CMD;
      xTaskNotify(motorTaskHandle, motorActionCommand, eSetValueWithOverwrite);
      break;
    case BLEMsgsEnum::msg_MotorStop:
      Serial.println("-- Received Msg: motorStop --");
      motorActionCommand.cmd = Commands_t::MOTOR_STOP_CMD;
      xTaskNotify(motorTaskHandle, motorActionCommand, eSetValueWithOverwrite);
      break;
  }
}

bool BleManager::isDeviceConnected() {
  return this->currentState == States::CONNECTED;
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

BleManager::BleManager() {
  Serial.println(">>>>>>>> BleManager() >>>>>>>>");

  this->init();
}

void BleManager::init() {
  Serial.println(">>>>>>>> BleManager::init() >>>>>>>>");
  
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

  this->currentState = States::START_ADVERTISING;
  
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
  xStatus = xQueueReceive(encoderReadQueue, &value, pdMS_TO_TICKS(10));
  if(xReturn == pdPASS) {
    encoderData = (EncoderData_t)(value);
    this->pTxCharacteristic->setValue(encoderData.absoluteStep);
    this->pTxCharacteristic->notify();
  }
}

void BleManager::runLoop() {
  for (;;)
  {
    switch(this->currentState) 
    {
      case States::START_ADVERTISING:
        Serial.println("--- States::START_ADVERTISING ---");
        this->startAdvertising();
        break;
      case States::LISTENING:
        Serial.println("--- States::LISTENING ---");
        if (deviceConnected && !oldDeviceConnected) {
          oldDeviceConnected = deviceConnected;
          this->currentState = States::CONNECTED;
        }
        break;
      case States::CONNECTED:
        Serial.println("--- States::CONNECTED ---");
        this->notifyEncoder();
        if(!lastReceivedMsg.empty()) {
          this->handleMsg(lastReceivedMsg);
        }
        break;
      case States::DISCONNECTED:
        Serial.println("--- States::DISCONNECTED ---");
        if (!deviceConnected && oldDeviceConnected) {
          oldDeviceConnected = deviceConnected;
          vTaskDelay(100);
          Serial.println("re-start advertising");
          this->currentState = States::START_ADVERTISING;
        }
        break;
    }
    vTaskDelay(1000);
  }
}
