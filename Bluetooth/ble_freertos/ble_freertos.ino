#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

/*--------------------------------------------------*/
/*---------------------- Callbacks ---------------------*/
/*--------------------------------------------------*/
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
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();
        Serial.println("*********");
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
    Serial.println("TaskBLEAdvertise - Loop Start ");
    
    Serial.println("TaskBLEscanAndWrite - 1 Sec Task Delay");
    vTaskDelay(1000); // Delay a second between loops.
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize serial communication at 115200 bits per second:
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

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}
