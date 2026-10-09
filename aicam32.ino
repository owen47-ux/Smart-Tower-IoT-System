#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// --- ตั้งค่า Wi-Fi ---
const char* ssid = "tanapark_104";      // ชื่อ Wi-Fi
const char* password = "00104even";   // รหัสผ่าน Wi-Fi

WebServer server(80);

// --- กำหนดพินสำหรับกล้อง AI Thinker ESP32-CAM ---
#define PWDN_GPIO_NUM     32
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
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define FLASH_LED_PIN      4 // ไฟ Flash LED บนบอร์ด

void handleCapture() {
  // ปิดไฟแฟลช ไม่ต้องเปิดสว่างแล้ว
  digitalWrite(FLASH_LED_PIN, LOW); 

  // ถ่ายภาพ
  camera_fb_t * fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera capture failed");
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  // ส่งไฟล์ภาพกลับไปให้ผู้เรียกใช้งาน
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);
  
  // คืนหน่วยความจำ
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);
  pinMode(FLASH_LED_PIN, OUTPUT);

  // ตั้งค่ากล้อง
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
  
  // แก้ไขชื่อพินสเปกใหม่ตรงนี้
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // ตั้งค่าขนาดรูปภาพ (VGA = 640x480 เหมาะกับ AI และส่งไว)
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // เริ่มต้นกล้อง
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // --- สั่งหมุนภาพ 180 องศาที่ตัวกล้อง ---
  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 1);    // กลับหัวแนวตั้ง
  s->set_hmirror(s, 1);  // สลับซ้ายขวาให้สมจริง

  // เชื่อมต่อ Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("Camera IP Address: ");
  Serial.println(WiFi.localIP()); // **จำ IP ตัวนี้ไว้ใช้งานใน Node-RED**

  // กำหนด Endpoint สำหรับสั่งถ่ายภาพ
  server.on("/capture", HTTP_GET, handleCapture);
  server.begin();
}

void loop() {
  server.handleClient();
}