
#include <esp32-hal-gpio.h>
#include <HardwareSerial.h>
#include <iotCmd.h>
#include <iotActuators.h>
#include <bleConfig.h> 

#include <serialHandler.h>
#include <atCommands.h>
#include <wifiInit.h>

// Binu Udayakumar binu@dronasys.com
// UI tools can be accessed at https://binuud.com

SerialHandler serialHandler(atCommands, sizeof(atCommands) / sizeof(atCommands[0]));

void setup() {
  
  Serial.begin(115200);
  Serial.println("Setup Begin...");
  delay(1000); // wait for serial monitor initialization
  // load preferences from EEPROM
  // this has to be called first before any other initiations
  // since we are storing pinout information here
  DeviceConfig config = devicePrefs.loadConfig();

  // now initialize IO devices with the device information loaded from above.
  initializeIODevices(devicePrefs.devices);

  // init wifi
  initWifi(devicePrefs.config.wifi_ssid, devicePrefs.config.wifi_password);

  serialHandler.help(); // print al the AT-Commands

  setupBle();

  Serial.println("Setup Done...");
  Serial.println("Listening for serial Input...");

}


// each module will have to implement its own loop.
// this loop will act like a scheduler without priority
// avoid calls to delay, from with module loops
void loop() {

  // listen for commands from serial interface 
  // and invoke the same
  serialHandler.loop();

  // run ble loop()
  loopBle();

  // run stepper, motors and servo
  loopActuator();
  // delay(10);

  simpleHelloWorld();
}
