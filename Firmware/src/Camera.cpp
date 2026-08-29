#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

const char* ssid = "Fios-2c3PM";
const char* password = "ink565jag85coop";

#define PWDN_PIN     32
#define RST_PIN      -1
#define XCLK_PIN      0
#define SIOD_PIN     26
#define SIOC_PIN     27
#define Y9_PIN       35
#define Y8_PIN       34
#define Y7_PIN       39
#define Y6_PIN       36
#define Y5_PIN       21
#define Y4_PIN       19
#define Y3_PIN       18
#define Y2_PIN        5
#define VSYNC_PIN    25
#define HREF_PIN     23
#define PCLK_PIN     22

httpd_handle_t server = NULL;

#define BOUNDARY_STR "123456789000000000000987654321"

static const char* streamType =
    "multipart/x-mixed-replace;boundary=" BOUNDARY_STR;

static const char* boundary =
    "\r\n--" BOUNDARY_STR "\r\n";

static const char* headerFmt =
    "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static esp_err_t streamHandler(httpd_req_t* req) {
  camera_fb_t* frame = NULL;
  esp_err_t err = ESP_OK;

  size_t dataLen = 0;
  uint8_t* data = NULL;
  char header[64];

  err = httpd_resp_set_type(req, streamType);

  if (err != ESP_OK) {
    return err;
  }

  while (true) {
    frame = esp_camera_fb_get();

    if (!frame) {
      Serial.println("Cam fail");
      err = ESP_FAIL;
    } else {
      dataLen = frame->len;
      data = frame->buf;
    }

    if (err == ESP_OK) {
      err = httpd_resp_send_chunk(
        req,
        boundary,
        strlen(boundary)
      );
    }

    if (err == ESP_OK) {
      size_t headerLen = snprintf(
        header,
        sizeof(header),
        headerFmt,
        dataLen
      );

      err = httpd_resp_send_chunk(
        req,
        header,
        headerLen
      );
    }

    if (err == ESP_OK) {
      err = httpd_resp_send_chunk(
        req,
        (const char*)data,
        dataLen
      );
    }

    if (frame) {
      esp_camera_fb_return(frame);
      frame = NULL;
      data = NULL;
    } else if (data) {
      free(data);
      data = NULL;
    }

    if (err != ESP_OK) {
      break;
    }
  }

  return err;
}

void startServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 81;

  httpd_uri_t uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = streamHandler,
    .user_ctx = NULL
  };

  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_register_uri_handler(server, &uri);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t camConfig;

  camConfig.ledc_channel = LEDC_CHANNEL_0;
  camConfig.ledc_timer = LEDC_TIMER_0;

  camConfig.pin_d0 = Y2_PIN;
  camConfig.pin_d1 = Y3_PIN;
  camConfig.pin_d2 = Y4_PIN;
  camConfig.pin_d3 = Y5_PIN;
  camConfig.pin_d4 = Y6_PIN;
  camConfig.pin_d5 = Y7_PIN;
  camConfig.pin_d6 = Y8_PIN;
  camConfig.pin_d7 = Y9_PIN;

  camConfig.pin_xclk = XCLK_PIN;
  camConfig.pin_pclk = PCLK_PIN;
  camConfig.pin_vsync = VSYNC_PIN;
  camConfig.pin_href = HREF_PIN;
  camConfig.pin_sccb_sda = SIOD_PIN;
  camConfig.pin_sccb_scl = SIOC_PIN;
  camConfig.pin_pwdn = PWDN_PIN;
  camConfig.pin_reset = RST_PIN;

  camConfig.xclk_freq_hz = 20000000;
  camConfig.pixel_format = PIXFORMAT_JPEG;

  camConfig.frame_size = FRAMESIZE_VGA;
  camConfig.jpeg_quality = 12;
  camConfig.fb_count = 2;

  esp_err_t initErr = esp_camera_init(&camConfig);

  if (initErr != ESP_OK) {
    Serial.printf("Init error 0x%x", initErr);
    return;
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected");

  startServer();

  Serial.print("Stream: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":81/stream");
}

void loop() {
  delay(10000);
}