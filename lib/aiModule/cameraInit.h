#ifndef CAMERA_INIT_H
#define CAMERA_INIT_H


static const char *TAG = "camera_httpd";

// #include "esp32-hal-ledc.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "camera_pins.h"






// Enable LED FLASH setting
#define CONFIG_LED_ILLUMINATOR_ENABLED 1



void initCamera();
static esp_err_t stream_handler(httpd_req_t *req);
esp_err_t options_handler(httpd_req_t *req);

void initCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

//   setupLedFlash(LED_GPIO_NUM);

}


httpd_handle_t camera_httpd = NULL;

esp_err_t options_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "*");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*"); // Add any custom headers your client might send
  httpd_resp_sendstr(req, ""); // Send an empty response for OPTIONS
  return ESP_OK;
}

static esp_err_t stream_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char part_buf[64];

    // Add CORS header to allow all origins
    // httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    // httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    // httpd_resp_set_status(req, "200");
    // httpd_resp_sendstr_chunk(req, "\n");

    // Add CORS header to allow all origins


    // Set multipart HTTP response headers
    res = httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
    
    if(res != ESP_OK) {
        return res;
    }
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"); // Enable CORS
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

        httpd_uri_t options_uri = {
            .uri = "/stream", // Or the specific URI your client targets
            .method = HTTP_OPTIONS,
            .handler = options_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &options_uri);

        
    }

    return server;
}


#endif // CAMERA_INIT_H