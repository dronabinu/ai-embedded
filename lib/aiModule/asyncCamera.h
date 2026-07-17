#ifndef ASYNC_CAMERA_SERVER_H
#define ASYNC_CAMERA_SERVER_H

#include "esp_log.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <iotCarFixedSteering.h>
#include <iotCmd.h>

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char* PARAM_INPUT_1 = "cmd";
const char* PARAM_INPUT_2 = "subCmd";
const char* PARAM_INPUT_3 = "identifier";
const char* PARAM_INPUT_4 = "value1";
const char* PARAM_INPUT_5 = "value2";

// Boundary definition for MJPEG stream chunking
#define PART_BOUNDARY "123456789000000000000987654321"


// Helper function to add CORS headers to a response
void addCORSHeaders(AsyncWebServerResponse *response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleStream(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginChunkedResponse(_STREAM_CONTENT_TYPE, [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
    static camera_fb_t * fb = NULL;
    static size_t _frame_len = 0;
    static size_t _frame_sent = 0;
    static enum { STREAM_START, STREAM_HEADER, STREAM_DATA, STREAM_END } _state = STREAM_START;
    static char header_buf[128];

    size_t outLen = 0;

    while (outLen < maxLen) {
      switch (_state) {
        case STREAM_START:
          fb = esp_camera_fb_get();
          if (!fb) {
            Serial.println("Camera capture failed");
            return 0; 
          }
          _frame_len = fb->len;
          _frame_sent = 0;
          _state = STREAM_HEADER;
          // fall through
          
        case STREAM_HEADER: {
          size_t blen = snprintf(header_buf, sizeof(header_buf), _STREAM_PART, _frame_len);
          size_t avail = maxLen - outLen;
          if (avail >= blen) {
            memcpy(buffer + outLen, header_buf, blen);
            outLen += blen;
            _state = STREAM_DATA;
          } else {
            return outLen; // Handle memory buffer constraints safely
          }
        } // fall through

        case STREAM_DATA: {
          size_t avail = maxLen - outLen;
          size_t remaining = _frame_len - _frame_sent;
          size_t toSend = (remaining < avail) ? remaining : avail;
          
          memcpy(buffer + outLen, fb->buf + _frame_sent, toSend);
          outLen += toSend;
          _frame_sent += toSend;

          if (_frame_sent == _frame_len) {
            esp_camera_fb_return(fb);
            fb = NULL;
            _state = STREAM_END;
          } else {
            return outLen;
          }
        } // fall through

        case STREAM_END: {
          size_t blen = strlen(_STREAM_BOUNDARY);
          size_t avail = maxLen - outLen;
          if (avail >= blen) {
            memcpy(buffer + outLen, _STREAM_BOUNDARY, blen);
            outLen += blen;
            _state = STREAM_START; 
          } else {
            return outLen;
          }
          break;
        }
      }
    }
    return outLen;
  });

  // Prevent browser caching issues
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

void initAsyncServer() {

    // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<div>This is a test</div>");
  });

  server.on("/update", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(204); // 204 No Content
        addCORSHeaders(response);
        request->send(response);
    });


  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {

    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) 
        && request->hasParam(PARAM_INPUT_2) 
         && request->hasParam(PARAM_INPUT_3) 
          && request->hasParam(PARAM_INPUT_4) 
           && request->hasParam(PARAM_INPUT_5) 
    ) {
      String inpCmd = request->getParam(PARAM_INPUT_1)->value();
      String subCmd = request->getParam(PARAM_INPUT_2)->value();
      String identifier = request->getParam(PARAM_INPUT_3)->value();
      String value1 = request->getParam(PARAM_INPUT_4)->value();
      String value2 = request->getParam(PARAM_INPUT_5)->value();
      Serial.printf("GPIO: cmd %s : subcmd %s id (%s) val1:(%s) val2:(%s)\n", subCmd, subCmd, identifier, value1, value2);

        IotCommand cmd;
        cmd.cmd = static_cast<DeviceCategory>(inpCmd.toInt());
        cmd.subcmd = static_cast<SubCmdEnum>(subCmd.toInt());
        cmd.identifier = identifier.toInt();
        cmd.value1 = value1.toInt();
        cmd.value2 = value2.toInt();
         debugIotCommand(&cmd);
        controlpadWithSpeed(&cmd);
    }
    else {

    }
    

    
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"success\"}");
    addCORSHeaders(response);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  server.on("/stream", HTTP_GET, [](AsyncWebServerRequest *request){
    handleStream(request);
  });
 

  // Start server
  server.begin();
}

#endif // ASYNC_CAMERA_SERVER_H