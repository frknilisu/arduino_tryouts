#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "MotorManager.h"
//#include "MissionController.h"

#define SERVICE_UUID_ENCODER        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_ENCODER "33d14ca1-1ba0-4247-bb85-0ea4504eb03d"

#define SERVICE_UUID_UART      "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MissionController;

class BleManager {
  public:
    BleManager(MotorManager* motorManager);
    void runLoop();

    void handleMsg(std::string receivedMsg);
    bool isDeviceConnected();
  private:
    enum class States {
      INITIALIZING,
      START_ADVERTISING,
      LISTENING,
      CONNECTED,
      DISCONNECTED
    };
    
    States currentState;
    BLEServer* pServer = NULL;
    BLEService* pService = NULL;
    BLECharacteristic* pTxCharacteristic = NULL;
    BLECharacteristic* pRxCharacteristic = NULL;

    MotorManager* motorManager;

    void initBLE();
    void startAdvertising();
    void notify();  
};


#endif
