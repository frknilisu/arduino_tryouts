#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include "Init.h"
#include <Arduino.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define SERVICE_UUID_UART      "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class BleManager {
  public:
    BleManager();
    void runLoop();

    void handleMsg(std::string receivedMsg);
    bool isDeviceConnected();
  private:
    enum class States {
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

    void init();
    void startAdvertising();
    void notifyEncoder();  
};


#endif
