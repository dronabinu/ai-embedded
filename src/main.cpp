
#define CAMERA_ENABLED 1 // if camera is enabled set to 1, else set to 0

#include "esp_http_server.h"
#include <esp32-hal-gpio.h>
#include <HardwareSerial.h>
#include <iotCmd.h>
#include <iotActuators.h>
#include <bleConfig.h> 

#include <serialHandler.h>
#include <atCommands.h>
#include <wifiInit.h>
#include <cameraInit.h>
#include "esp_log.h"
#include "esp_camera.h"

static const char *TAG = "camera_httpd";

// Binu Udayakumar binu@dronasys.com
// UI tools can be accessed at https://binuud.com

SerialHandler serialHandler(atCommands, sizeof(atCommands) / sizeof(atCommands[0]));

httpd_handle_t camera_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char part_buf[64];

    // Set multipart HTTP response headers
    res = httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
    if(res != ESP_OK) {
        return res;
    }

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
        } else {
            _jpg_buf = fb->buf;
            _jpg_buf_len = fb->len;

            size_t hlen = snprintf(part_buf, 64, 
                "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", (uint32_t)_jpg_buf_len);

            // Send multipart frame header
            res = httpd_resp_send_chunk(req, part_buf, hlen);
            if(res == ESP_OK){
                // Send JPEG frame
                res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
            }
            if(res == ESP_OK){
                // Send line break after frame
                res = httpd_resp_send_chunk(req, "\r\n", 2);
            }
            esp_camera_fb_return(fb);
            if(res != ESP_OK){
                break;
            }
        }
    }
    return res;
}

static esp_err_t index_handler(httpd_req_t *req){
    const char* html = "<html><body><h1>ESP32-CAM Stream</h1><img src=\"/stream\"/></body></html>";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html, strlen(html));
    return ESP_OK;
}


static esp_err_t hello_handler(httpd_req_t *req){
    const char* html = "<html><body><h1>Hello World</h1></body></html>";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html, strlen(html));
    return ESP_OK;
}


static httpd_handle_t start_camera_server(){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK){
        httpd_uri_t index_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = index_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &index_uri);

        httpd_uri_t hello_uri = {
            .uri = "/hello",
            .method = HTTP_GET,
            .handler = hello_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &hello_uri);

        httpd_uri_t stream_uri = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = stream_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &stream_uri);
    }

    return server;
}

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

  Serial.println("Setting Bluetooth...");
  setupBle();

  // if CAMERA is enabled, init the camera
#if CAMERA_ENABLED 
  Serial.println("Setting Camera...");
  initCamera();
#endif

  camera_httpd = start_camera_server();

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
  
}
