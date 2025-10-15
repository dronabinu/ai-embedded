#ifndef IOT_BOARD_STATE_H
#define IOT_BOARD_STATE_H

// maintain state information on GPIO pinouts persistant on restarts
// Binu Udayakumar (binu@dronasys.com)
// Json was tried, there was a considerable slowdown. Also I want to keep the image size low.
// This uses the preference library of espressif
// https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/preferences.html

#include <Preferences.h>
#include <iotCmd.h>
#include <helpers.h>

#define DEFAULT_CONFIG "DC"
#define DEFAULT_BLE_PASSWORD "brain123"

#define DELIMITER ","
#define MAX_ITEMS 20

struct ConnectedIO {
    DeviceCategory dev;
    String id;
    int pin[10];
    int pinCnt;
};

struct DeviceConfig {
    String id;                      // name given to device
    String wifi_ssid;               
    String wifi_password;
    String ble_password;            // TBD, way to secure BLE cnnection 
    String device_names[MAX_ITEMS];   // list of connectect IO to the ESP32
    size_t dev_count; // number of devices
};

class DevicePreference {
private:
    Preferences preferences;
    String namespaceName;

public:
    DeviceConfig config;
    ConnectedIO devices[MAX_ITEMS];

    DevicePreference(const char* ns) {
        config = {"", "", "", "", {}, 0};
        namespaceName = String(ns);
    }

    void begin() {
        preferences.begin(namespaceName.c_str(), false);
    }

    void end() {
        preferences.end();
    }

    void clear() {
        begin();
        preferences.clear();
        end();
    }

    void saveWifi() {
        begin();
        preferences.putString("wifi_ssid", config.wifi_ssid);
        preferences.putString("wifi_password", config.wifi_password);
        end();
    }

    void saveBle() {
        begin();
        preferences.putString("ble_password", config.ble_password);
        end();
    }

    void saveDeviceNames() {
        String connectedDevices = joinStringArray(config.device_names, config.dev_count, ',');
        begin();
        preferences.putInt("dev_count", config.dev_count);
        preferences.putString("device_names", connectedDevices);
        end();
    }

    void printDevice(ConnectedIO& io) {
        Serial.printf("Device Name %s, type %d, pin count %d ", io.id, io.dev, io.pinCnt);
        for(int i;i<io.pinCnt;i++) {
            Serial.printf(" %d ", io.pin[i]);
        }
        Serial.println();
    }


    DeviceConfig loadConfig() {

        begin();
        config.ble_password = preferences.getString("ble_password", "");
        config.wifi_ssid = preferences.getString("wifi_ssid", "");
        config.wifi_password = preferences.getString("wifi_password", "");
        config.dev_count = preferences.getInt("dev_count", 0);

        String connectedDevices = preferences.getString("device_names", "");

        if (connectedDevices != "") {
            String parts[config.dev_count]; // adjust size as needed
            splitString(connectedDevices, ',', parts, config.dev_count);
            for (int i = 0; i < config.dev_count; i++) {
                config.device_names[i] = parts[i];
                Serial.printf("Trying to load config for %s\n", parts[i]);
                ConnectedIO loadedIO;
                loadIODevice(parts[i], loadedIO);
                devices[i] = loadedIO;
                printDevice(loadedIO);
            }
        }


        end();
        return config;

    }


    // Save an array of ConnectedIO structs
    bool saveIODevice(ConnectedIO io) {

        Serial.printf("Trying to add/edit device %s\n", io.id);
        // check if io device exists in connected devices
        int deviceFound = -1;
        for (int i = 0; i < config.dev_count; i++) {
            if (config.device_names[i] == io.id) {
                deviceFound = i;
                break;
            }
        }

        if (deviceFound == -1) {
            // add the device to the device_names and save
            if (config.dev_count < MAX_ITEMS) {
                config.device_names[config.dev_count] = io.id;
                deviceFound = config.dev_count;
                config.dev_count++;
                saveDeviceNames();

                
            } else {
                Serial.printf("Cannot add more than %d io devices", MAX_ITEMS);
                return false; // array full
            }
        }
        
        for (int i = 0; i < config.dev_count; i++) {
            devices[deviceFound].pin[i] = io.pin[i];
        }
        devices[deviceFound].pinCnt = io.pinCnt;

        begin();
        size_t whatsLeft = preferences.freeEntries();  
        Serial.printf("Free keys left %u\n", whatsLeft);
        String prefix = io.id;

        preferences.putInt((prefix + "_dev").c_str(), (int)io.dev);
        preferences.putString((prefix + "_id").c_str(), io.id);
        preferences.putBytes((prefix + "_pin").c_str(), io.pin, sizeof(io.pin));
        preferences.putInt((prefix + "_pinCnt").c_str(), io.pinCnt);
        end();

        return true;
    }


    bool loadIODevice(String deviceName, ConnectedIO& io) {
        
        String prefix = deviceName;
        begin();

        io.dev = (DeviceCategory)preferences.getInt((prefix + "_dev").c_str(), 0);
        io.id = preferences.getString((prefix + "_id").c_str(), "");
        size_t bytesRead = preferences.getBytes((prefix + "_pin").c_str(), io.pin, sizeof(io.pin));
        io.pinCnt = preferences.getInt((prefix + "_pinCnt").c_str(), 0);

        end();
        return true;
    }

};


DevicePreference devicePrefs("b");

#endif // IOT_BOARD_STATE_H