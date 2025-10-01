#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp32-hal-gpio.h>
#include <HardwareSerial.h>
#include <iotCmd.h>
#include <iotActuators.h>
#include <bleConfig.h> 

#include <serialCmd.h>


uint8_t txValue = 0;


void setup() {
  
  Serial.begin(115200);
  Serial.println("Setup Begin...");

  DeviceConfig loadedDevice = devicePrefs.loadConfig();
  configPins();

  myStepper.setSpeed(15); // speed in RPM

  setupBle();

  Serial.println("Setup Done...");
  Serial.println("Listening for serial Input...");
}


void loop() {

  // listen for commands from serial interface 
  // and invoke the same
  loopSerialCmd();

  // run ble loop()
  loopBle();

  // run stepper, motors and servo
  loopActuator();
  delay(100);
}
