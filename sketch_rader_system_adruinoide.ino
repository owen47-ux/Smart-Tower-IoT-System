#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// --- ตั้งค่า Wi-Fi & MQTT Server ---
const char* ssid = "tanapark_104";
const char* password = "00104even";
const char* mqtt_server = "192.168.1.9"; // IP เครื่อง Node-RED / MQTT Broker

// --- กำหนดพินอุปกรณ์ ---
#define SERVO_PIN 13
#define TRIG_PIN  12
#define ECHO_PIN  14
#define BUZZER_PIN 25
#define LED_PIN    33

#define DISTANCE_THRESHOLD 30 // ระยะเตือนภัย (เซนติเมตร)

Servo radarServo;
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastReconnectAttempt = 0;

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // Timeout 25ms
  if (duration == 0) return 400; // หากจับระยะไม่ได้ ให้คืนค่าไกลสุด
  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  radarServo.attach(SERVO_PIN);
  
  // เชื่อมต่อ Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  client.setServer(mqtt_server, 1883);
}

// ระบบเชื่อมต่อ MQTT แบบไม่ทำให้การหมุนเรดาร์ค้างเมื่อเน็ตหลุด
boolean reconnect() {
  if (client.connect("ESP32_Radar_Client")) {
    return true;
  }
  return false;
}

void processRadarScan(int angle) {
  radarServo.write(angle);
  delay(30); // รอให้มอเตอร์เข้าตำแหน่ง
  
  long distance = getDistance();
  bool alert = (distance <= DISTANCE_THRESHOLD && distance > 0);

  // ควบคุมการแจ้งเตือน Local (Buzzer + LED)
  if (alert) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }

  // ส่งข้อมูลออก Serial สำหรับโปรแกรม Processing
  Serial.print(angle);
  Serial.print(",");
  Serial.print(distance);
  Serial.print(".");

  // ส่งข้อมูลรูปแบบ JSON ไปยัง Node-RED ผ่าน MQTT Topic: fortress/radar
  if (client.connected()) {
    String payload = "{\"angle\":" + String(angle) + 
                     ",\"distance\":" + String(distance) + 
                     ",\"alert\":" + String(alert ? "true" : "false") + "}";
    client.publish("fortress/radar", payload.c_str());
  }
}

void loop() {
  // ตรวจสอบการเชื่อมต่อ MQTT โดยไม่บล็อกการทำงานหลัก
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) { // ลองต่อใหม่ทุกๆ 5 วินาที
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }

  // กวาดจาก 15 ไป 165 องศา
  for (int i = 15; i <= 165; i += 2) {
    processRadarScan(i);
  }
  // กวาดกลับจาก 165 มา 15 องศา
  for (int i = 165; i >= 15; i -= 2) {
    processRadarScan(i);
  }
}