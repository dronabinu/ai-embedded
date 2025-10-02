// for parsing commands from serial and invoking same
// using the AT+ command structure
// AT+[CMD]? for reading values
// AT+[CMD]=param1, param2 for setting values
// Binu Udayakumar (binu@dronasys.com)


#include <boardState.h>
#include <iotActuators.h>
#include <helpers.h>

// Struct for command and its callback

typedef void (*CmdCallback)(const String& args);
typedef void (*CmdReadCallback)();

// Forward declarations of command callbacks

void atCmdWifiSSID(const String& args);
void atCmdWifiPass(const String& args);

void atCmdStepperConfig(const String& args);
void atCmdServoConfig(const String& args);

void atCmdLedConfig(const String& args);
void atCmdConfig(const String& args);

void atCmdStepperAngle(const String& args);
void atCmdServoAngle(const String& args);

void atCmdClearStorage(const String& args);

void atReadWifiSSID();
void atReadWifiPass();

void atReadStepperConfig();
void atReadServoConfig();

void atReadLedConfig();
void atReadConfig();

void atReadStepperAngle();
void atReadServoAngle();


void loopSerialCmd();
int splitString(const String& input, char delimiter, String output[], int maxParts);


struct AtCommand {
    const char* name;   // the actual AT command without the suffix and prefix
    const char* desc;   // description of the command, that will be displayed to the user
    CmdCallback execCallback;       // invoked when the command is called with suffix "=", for configuring the device
    CmdReadCallback readCallback;   // invoked when the command is called in read mode, with suffix "?", there are no parameters in read mode
};

// Preset array of commands and their callbacks
AtCommand atCommands[] = {
    
    {"WIFI_SSID", "Config Wifi SSID, eg: AT_CONF_WIFI_SSID=[WIFI-SSID]", atCmdWifiSSID, atReadWifiSSID},
    {"WIFI_PASS", "Config Wifi SSID, eg: AT_CONF_WIFI_PASS=[WIFI-PASS]", atCmdWifiPass, atReadWifiPass},
    {"STEPPER", "Config Stepper Pin, eg: AT_CONF_STP_PIN=[STEPPER NUMBER], [STEPPER_PIN, DIR_PIN]" , atCmdStepperConfig, atReadStepperConfig},
    {"SERVO", "Config Stepper Pin, eg: AT_CONF_STP_PIN=[STEPPER NUMBER], [STEPPER_PIN, DIR_PIN]" , atCmdServoConfig, atReadServoConfig},
    {"LED", "Config Led Pin, eg: AT_CONF_LED_PIN=[LED NUMBER], [LED_PIN]" , atCmdLedConfig, atReadLedConfig},
    {"STP_ANGLE", "Move stepper to angle, eg: move stepper 1 to angle 20, AT+STP_ANGLE=1,20", atCmdStepperAngle, atReadStepperAngle},
    {"SRV_ANGLE", "Move servo to angle, eg: move servo 1 to angle 20, AT+STP_ANGLE=1,20", atCmdServoAngle, atReadServoAngle},
    {"CLEAR_STORAGE", "Clear all stored values", atCmdClearStorage, nullptr}
};

const int atCommandCount = sizeof(atCommands) / sizeof(atCommands[0]);

void loopSerialCmd() {

    if (Serial.available() > 0) {
        
        String input = Serial.readStringUntil('\n');
        input.trim();

        String prefix = input.substring(0, 4);  // Get the first two characters
        prefix.toUpperCase();   

        if (!prefix.startsWith("AT")) {
            Serial.println("ERROR: Commands must start with 'AT'");
            return;
        }

        // Handle help command if input is exactly "AT?" or "AT+?"
        if (prefix == "AT?" || prefix == "AT+?") {
            Serial.println("Available commands:");
            Serial.println("  AT? -- help message");
            for (int i = 0; i < atCommandCount; i++) {
                Serial.println(String("  AT+") + atCommands[i].name + "=<params>");
                Serial.println(String("    Desc: ") + atCommands[i].desc);
            }
            Serial.println("OK");
            return;
        }

        // Remove the "AT" prefix
        String cmdLine = input.substring(3);
        cmdLine.trim();

        if (cmdLine.length() == 0) {
            Serial.println("OK");
            return;  // Just "AT" command, respond OK
        }
        cmdLine.toUpperCase();  // Convert cmdName to lowercase
        // Find command in atCommands array
        bool handled = false;
        for (int i = 0; i < atCommandCount; i++) {
            String cmdName = atCommands[i].name;

            if (cmdLine.startsWith(cmdName)) {
                String params = input.substring(3+cmdName.length()); // to prevent auto uppercase, done above to influence the parameters
                params.trim();

                // If parameters start with '=', remove it and pass rest as params
                if (params.startsWith("=")) {
                    params = params.substring(1);
                    params.trim();
                    if (atCommands[i].execCallback != nullptr) {
                        atCommands[i].execCallback(params);
                    }
                    
                    handled = true;
                } else {
                    // No parameters part after command
                    // treat it as a read command
                    if (atCommands[i].readCallback != nullptr) {
                        atCommands[i].readCallback();
                    }
                    
                    handled = true;
                }

                break;
            }
        }

        if (!handled) {
            Serial.println("Unknown command. Available commands: ANGLE, WIFI_SSID, WIFI_PASS, CONFIG.");
        }
    }    
}




// AT command handlers

void atCmdAngle(const String& params) {
    int angle = params.toInt();
    if (angle >= 0 && angle <= 360) {
        controlNemaStepper(1, angle);
        Serial.println("OK");
    } else {
        Serial.println("ERROR: Invalid angle (0-360)");
    }
}

void atCmdWifiSSID(const String& params) {
    if (params.length() > 0) {
        devicePrefs.config.wifi_ssid = params;
        devicePrefs.saveWifi();
        Serial.printf("atCmdWifiSSID new ssid  %s , OK\n", params);
    } else {
        Serial.println("ERROR: SSID required");
    }
}

void atCmdWifiPass(const String& params) {
    if (params.length() > 0) {
        devicePrefs.config.wifi_password = params;
        devicePrefs.saveWifi();
        Serial.printf("atCmdWifiPass new password %s %s , OK\n", params);
    } else {
        Serial.println("ERROR: Password required");
    }
}

void atCmdStepperConfig(const String& params) {

    if (params.length() > 0) {
        String parts[2]; // adjust size as needed
        Serial.printf("Configuring stepper start");
        int numParts = splitString(params, ',', parts, 3);
        if (numParts == 3) {
            
            int stepper_id = parts[0].toInt();
            int step_pin = parts[1].toInt();
            int dir_pin = parts[2].toInt();
            String stepperName = "STP" + String(stepper_id);
            ConnectedIO io = {DeviceCategory_stepper, stepperName, {step_pin, dir_pin}, 2};
            devicePrefs.saveIODevice(io);
            devicePrefs.printDevice(io);
            configNemaStepper(step_pin, dir_pin);
        } else {
            Serial.println("ERROR: Invalid format");
        }
    } else {
        Serial.println("ERROR: invalid params, format stepper, gpio");
    }
}

void atCmdServoConfig(const String& params) {

    if (params.length() > 0) {
        String parts[2]; // adjust size as needed
        Serial.printf("Configuring servo");
        int numParts = splitString(params, ',', parts, 2);
        if (numParts == 2) {
            
            int servo_id = parts[0].toInt();
            int servo_Pin = parts[1].toInt();
            String servoName = "SRV" + String(servo_id);
            ConnectedIO io = {DeviceCategory_servo, servoName, {servo_Pin}, 1};

            devicePrefs.saveIODevice(io);
            devicePrefs.printDevice(io);
            
            configServo(servo_id, servo_Pin);

        } else {
            Serial.println("ERROR: Invalid format");
        }
    } else {
        Serial.println("ERROR: invalid params, format stepper, gpio");
    }
}

void atCmdConfig(const String& params) {
    if (params.length() > 0) {
        Serial.println("atCmdConfig OK");
    } else {
        Serial.println("ERROR: Config parameters required");
    }
}

void atCmdLedConfig(const String& params) {
    if (params.length() > 0) {
        int ledPin = params.toInt();
        String stepperName = "LED1";
        ConnectedIO io = {DeviceCategory_led, stepperName, {ledPin}, 1};
        devicePrefs.saveIODevice(io);
        devicePrefs.printDevice(io);
    } else {
        Serial.println("ERROR: invalid params, format stepper, angle");
    }
}


void atCmdStepperAngle(const String& params) {
    if (params.length() > 0) {
        String parts[2]; // adjust size as needed
        int numParts = splitString(params, ',', parts, 2);
        if (numParts == 2) {
            int stepper = parts[0].toInt();
            int angle = parts[1].toInt();
            if (angle >= 0 && angle <= 360) {
                controlNemaStepper(stepper, angle);
                Serial.println("OK");
            } else {
                Serial.println("ERROR: Invalid angle (0-360)");
            }
        } else {
            Serial.println("ERROR: Invalid format");
        }
    } else {
        Serial.println("ERROR: invalid params, format stepper, angle");
    }
}

void atCmdServoAngle(const String& params) {
    if (params.length() > 0) {
        String parts[2]; // adjust size as needed
        int numParts = splitString(params, ',', parts, 2);
        if (numParts == 2) {
            int servo = parts[0].toInt();
            int angle = parts[1].toInt();
            if (angle >= 0 && angle <= 360) {
                controlServo(servo, angle);
                Serial.println("OK");
            } else {
                Serial.println("ERROR: Invalid angle (0-360)");
            }
        } else {
            Serial.println("ERROR: Invalid format");
        }
    } else {
        Serial.println("ERROR: invalid params, format stepper, angle");
    }
}

void atCmdClearStorage(const String& params) {
    devicePrefs.clear();
    Serial.println("Cleared storage");
}

void atReadStepperAngle() {
    Serial.println("Stepper angle : ");    
}

void atReadServoAngle() {
    Serial.println("Stepper angle : ");    
}

void atReadWifiSSID() {
    Serial.printf("Wifi ssid : %s \n", devicePrefs.config.wifi_ssid);    
}


void atReadWifiPass() {
    Serial.printf("Wifi Password : %s \n", devicePrefs.config.wifi_password);    
}

void atReadServoConfig() {
    for(int i=0;i<MAX_ITEMS;i++) {
        ConnectedIO io = devicePrefs.devices[i];
        if (io.dev == DeviceCategory_servo) {
            devicePrefs.printDevice(io);
        }
    }
}

void atReadStepperConfig() {
    for(int i=0;i<MAX_ITEMS;i++) {
        ConnectedIO io = devicePrefs.devices[i];
        if (io.dev == DeviceCategory_stepper) {
            devicePrefs.printDevice(io);
        }
    }
}

void atReadLedConfig() {
    for(int i=0;i<MAX_ITEMS;i++) {
        ConnectedIO io = devicePrefs.devices[i];
        if (io.dev == DeviceCategory_led) {
            devicePrefs.printDevice(io);
        }
    }
}

void atReadConfig() {
    Serial.print("Stepper angle : \n");    
}

