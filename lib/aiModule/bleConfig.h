#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <iotCmd.h>

#define SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

uint8_t txValue = 0;

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool ble_deviceConnected = false;
bool ble_oldDeviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    ble_deviceConnected = true;
    Serial.println("BLE Client connected");
  };

  void onDisconnect(BLEServer *pServer) {
    ble_deviceConnected = false;
    Serial.println("BLE Client disconnected");
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    
    const uint8_t* data = pCharacteristic->getData();       // get raw data pointer

    IotCommand cmd;
    decodeCommand(data, &cmd);
    // debugIotCommand(&cmd);
    
    if (cmd.cmd == DeviceCategory_servo) {
      // handle servo
      controlServo(cmd.identifier, cmd.value1);
    } else if (cmd.cmd == DeviceCategory_move) {
      controlpadWithSpeed(&cmd);
    } else if (cmd.cmd == DeviceCategory_led) {
      controlLed(cmd.identifier, cmd.value1);
    } else if (cmd.cmd == DeviceCategory_stepper) {
      // handle stepper
      // controlStepper(cmd.identifier, cmd.value1);
      controlNemaStepper(cmd.identifier, cmd.value1);
    } 
    
  }
};


void setupBle() {

  // if (bleName.length() == 0) {
  //   bleName = "BrainCamRover";
  //   Serial.printf("Ble name not set, using %s", bleName);
  // }
  // Create the BLE Device
  BLEDevice::init("BrainCamRover");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);

  // Descriptor 2902 is not required when using NimBLE as it is automatically added based on the characteristic properties
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.printf("BLE Service id %s, characteristic %s", SERVICE_UUID, CHARACTERISTIC_UUID_RX);
  Serial.println("Waiting a client connection to notify...");
}

void loopBle() {
  if (ble_deviceConnected) {
    // Serial.print("Notifying Value: ");
    // Serial.println(txValue);
    // pTxCharacteristic->setValue(&txValue, 1);
    // pTxCharacteristic->notify();
    // txValue++;
    // delay(1000);  // Notifying every 1 second
  }

  // disconnecting
  if (!ble_deviceConnected && ble_oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("Started advertising again...");
    ble_oldDeviceConnected = false;
  }
  // connecting
  if (ble_deviceConnected && !ble_oldDeviceConnected) {
    // do stuff here on connecting
    ble_oldDeviceConnected = true;
  }
}