
#define CAMERA_ENABLED 1 // if camera is enabled set to 1, else set to 0
#define BLUETOOTH_ENABLED 1 // if bluetooth is enabled

#include <HardwareSerial.h>
#include <iotCmd.h>
#include <iotActuators.h>

#if BLUETOOTH_ENABLED 
#include <bleConfig.h> 
#endif

#include <serialHandler.h>
#include <atCommands.h>
#include <wifiInit.h>

#if CAMERA_ENABLED 
#include <cameraInit.h>
#endif



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
  Serial.println("Loading Prefs...");
  DeviceConfig config = devicePrefs.loadConfig();

  // now initialize IO devices with the device information loaded from above.
  Serial.println("Initializing Devices...");
  initializeIODevices(devicePrefs.devices);

  // init wifi
  Serial.println("Initializing wifi...");
  initWifi(devicePrefs.config.wifi_ssid, devicePrefs.config.wifi_password);

  serialHandler.help(); // print al the AT-Commands

#if BLUETOOTH_ENABLED    
  Serial.println("Setting Bluetooth...");
  setupBle();
#endif

  // if CAMERA is enabled, init the camera
#if CAMERA_ENABLED 
  Serial.println("Setting Camera...");
  initCamera();
  camera_httpd = start_camera_server();
#endif

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
#if BLUETOOTH_ENABLED   
  loopBle();
#endif  

  // run stepper, motors and servo
  loopActuator();
  
}
