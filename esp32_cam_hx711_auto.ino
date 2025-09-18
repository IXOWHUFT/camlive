/* esp32_cam_hx711_auto.ino (simplified)
   - captures JPEG, uploads to upload.php (binary)
   - notifies notify.php with {filename, url, weight, stream, timestamp}
   - starts camera server for live stream at http://<esp_ip>/stream
*/

#include "esp_camera.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "HX711.h"
#include "base64.h" // optional if needed

// CONFIG
const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASS = "YOUR_PASS";
const char* SERVER_UPLOAD_URL = "http://YOUR_SERVER/esp_project/upload.php";
const char* SERVER_NOTIFY_URL = "http://YOUR_SERVER/esp_project/notify.php";

const float WEIGHT_THRESHOLD = 60.0;

// HX711 pins
#define DT_PIN  4
#define SCK_PIN 15
HX711 scale;

// camera pins (AI-Thinker)
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void startCameraServer(); // provided by example code / camera_server
void initCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0; config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM; config.pin_d2 = Y4_GPIO_NUM; config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM; config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM; config.pin_pclk = PCLK_GPIO_NUM; config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM; config.pin_sscb_sda = SIOD_GPIO_NUM; config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000; config.pixel_format = PIXFORMAT_JPEG;
  if(psramFound()){ config.frame_size = FRAMESIZE_SVGA; config.jpeg_quality = 10; config.fb_count = 2; }
  else { config.frame_size = FRAMESIZE_VGA; config.jpeg_quality = 12; config.fb_count = 1; }
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) { Serial.printf("Camera init failed: 0x%x\n", err); while(true) delay(1000); }
}

String uploadToServer(camera_fb_t * fb) {
  HTTPClient http;
  http.begin(SERVER_UPLOAD_URL);
  http.addHeader("Content-Type", "image/jpeg");
  int httpCode = http.sendRequest("POST", (uint8_t*)fb->buf, fb->len);
  String result = "";
  if (httpCode == 200) result = http.getString();
  http.end();
  return result;
}

bool notifyServer(const String &filename, const String &url, float weight) {
  HTTPClient http;
  http.begin(SERVER_NOTIFY_URL);
  http.addHeader("Content-Type", "application/json");
  String streamUrl = String("http://") + WiFi.localIP().toString() + String(":81/stream"); // typical path
  unsigned long ts = millis()/1000;
  String payload = "{\"filename\":\"" + filename + "\",\"url\":\"" + url + "\",\"weight\":" + String(weight) + ",\"stream\":\"" + streamUrl + "\",\"timestamp\":" + String(ts) + "}";
  int code = http.POST(payload);
  if(code == 200) {
    Serial.println("Notify OK");
    http.end();
    return true;
  } else {
    Serial.printf("Notify fail: %d\n", code);
    http.end();
    return false;
  }
}

void setup(){
  Serial.begin(115200);
  scale.begin(DT_PIN, SCK_PIN); scale.set_scale(); scale.tare();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting WiFi");
  unsigned long start = millis();
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("."); delay(500);
    if(millis() - start > 20000) break;
  }
  Serial.println(); Serial.print("IP: "); Serial.println(WiFi.localIP());

  initCamera();
  startCameraServer(); // provide stream at /stream
}

unsigned long lastShot = 0;
const unsigned long shotCooldown = 5000;

void loop(){
  float weight = scale.get_units(5);
  Serial.printf("Weight: %.2f\n", weight);
  if(weight >= WEIGHT_THRESHOLD && (millis() - lastShot) > shotCooldown){
    lastShot = millis();
    camera_fb_t * fb = esp_camera_fb_get();
    if(!fb) { Serial.println("capture failed"); return; }
    String srvResp = uploadToServer(fb);
    Serial.println("Upload response: " + srvResp);
    // try parse JSON {"url":"...","filename":"..."}
    String filename = "";
    String url = "";
    if(srvResp.indexOf("{") >= 0){
      int p = srvResp.indexOf("\"filename\"");
      if(p>=0){
        int q = srvResp.indexOf(":", p);
        int a = srvResp.indexOf("\"", q+1);
        int b = srvResp.indexOf("\"", a+1);
        if(a>=0 && b>a) filename = srvResp.substring(a+1,b);
      }
      p = srvResp.indexOf("\"url\"");
      if(p>=0){
        int q = srvResp.indexOf(":", p);
        int a = srvResp.indexOf("\"", q+1);
        int b = srvResp.indexOf("\"", a+1);
        if(a>=0 && b>a) url = srvResp.substring(a+1,b);
      }
    }
    if(filename.length() == 0) filename = "photo_" + String(millis()) + ".jpg";
    if(url.length() == 0) url = String("http://yourserver/uploads/") + filename;
    notifyServer(filename, url, weight);
    esp_camera_fb_return(fb);
  }
  delay(500);
}
