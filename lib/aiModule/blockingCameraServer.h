#ifndef BLOCKING_CAMERA_SERVER_H
#define BLOCKING_CAMERA_SERVER_H


// This http server sends stream on /stream
// But the /stream will block any other activity
// other than serial controls

static const char *TAG = "camera_httpd";

// #include "esp32-hal-ledc.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_http_server.h"


httpd_handle_t camera_httpd = NULL;
static esp_err_t stream_handler(httpd_req_t *req);
esp_err_t options_handler(httpd_req_t *req);

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

static esp_err_t carcontrol_handler(httpd_req_t *req){

    char buf[128];
    char dir_param[32] = "stop";
    char speed_param[32] = "255";
    int speedVal = 255;

    // Read the query string from the URL
    if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
        // Extract 'dir' parameter
        if (httpd_query_key_value(buf, "dir", dir_param, sizeof(dir_param)) != ESP_OK) {
            strcpy(dir_param, "stop");
        }
        // Extract 'speed' parameter
        if (httpd_query_key_value(buf, "speed", speed_param, sizeof(speed_param)) == ESP_OK) {
            speedVal = atoi(speed_param);
            speedVal = constrain(speedVal, 0, 255);
        }
    }
    Serial.printf("Dir and speed %s:%s", dir_param, speed_param);
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

        httpd_uri_t stream_options_uri = {
            .uri = "/stream", // Or the specific URI your client targets
            .method = HTTP_OPTIONS,
            .handler = options_handler,
            .user_ctx = NULL
        };

        // for car controls
        httpd_register_uri_handler(server, &stream_options_uri);

        httpd_uri_t car_control_uri = {
            .uri = "/carcontrol",
            .method = HTTP_GET,
            .handler = carcontrol_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &car_control_uri);

        httpd_uri_t car_control_options_uri = {
            .uri = "/carcontrol", // Or the specific URI your client targets
            .method = HTTP_OPTIONS,
            .handler = options_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &car_control_options_uri);
        
    }

    return server;
}

#endif // BLOCKING_CAMERA_SERVER_H