#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// --- กำหนดค่า Wi-Fi และ MQTT Broker ---
const char* ssid = "tanapark_104";
const char* password = "00104even";
const char* mqtt_server = "192.168.1.9"; 

// Topic สำหรับรับคำสั่งสั่งเปิดประตู (จาก Node-RED / n8n)
const char* topic_command = "home/door/command";
// Topic สำหรับส่งสถานะกลับไปแสดงบน Dashboard
const char* topic_status  = "home/door/status";

WiFiClient espClient;
PubSubClient client(espClient);

// --- กำหนดขาพินอุปกรณ์ ---
#define SS_PIN 2     // D4 (GPIO 2)
#define RST_PIN 0    // D3 (GPIO 0)
#define LED_RED 4    // D2 (GPIO 4)
#define LED_GREEN 15 // D8 (GPIO 15)
#define SERVO_PIN 5  // D1 (GPIO 5)

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myServo;

// รหัสคีย์การ์ดที่อนุญาต
byte allowedUID[] = {0x44, 0x4D, 0xCE, 0x06};

// ฟังก์ชันเปิดประตูและปิดอัตโนมัติ 5 วินาที (สำหรับคีย์การ์ด / สแกนหน้า / ปุ่มกดชั่วคราว)
void unlockDoor(String triggerSource) {
  Serial.println("Access Granted by: " + triggerSource);
  client.publish(topic_status, "UNLOCKED");
  
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
  myServo.write(90); // สั่งเซอร์โวเปิดกลอน
  
  delay(5000); // เปิดค้างไว้ 5 วินาที
  
  Serial.println("Closing Door Automatically...");
  myServo.write(0); // สั่งเซอร์โวล็อก
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);
  client.publish(topic_status, "LOCKED");
}

// ฟังก์ชันรับคำสั่งผ่าน MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("MQTT Command Received: ");
  Serial.println(message);

  // 1. คำสั่งเปิดประตูแบบปิดเองอัตโนมัติ 5 วินาที
  if (message == "UNLOCK" || message == "UNLOCK_AI") {
    unlockDoor("MQTT / AI Camera");
  } 
  // 2. คำสั่งเปิดประตูค้างไว้ตลอด (Hold Open Mode)
  else if (message == "HOLD") {
    Serial.println("Hold Open Mode Activated");
    myServo.write(90);           // หมุนเซอร์โวไปตำแหน่งเปิด
    digitalWrite(LED_GREEN, HIGH); // ไฟเขียวติด
    digitalWrite(LED_RED, LOW);   // ไฟแดงดับ
    client.publish(topic_status, "HOLD_OPEN");
  } 
  // 3. คำสั่งสั่งล็อกประตูทันที
  else if (message == "LOCK") {
    Serial.println("Door Locked Manually");
    myServo.write(0);            // หมุนเซอร์โวกลับตำแหน่งล็อก
    digitalWrite(LED_GREEN, LOW);  // ไฟเขียวดับ
    digitalWrite(LED_RED, HIGH);  // ไฟแดงติด
    client.publish(topic_status, "LOCKED");
  }
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected. IP: " + WiFi.localIP().toString());
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP8266Door-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe(topic_command); // คอยฟังคำสั่งเปิดประตู
      client.publish(topic_status, "LOCKED");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  myServo.attach(SERVO_PIN);
  myServo.write(0); 
  
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // ตรวจสอบคีย์การ์ด RFID (Manual Control)
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  bool accessGranted = true;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] != allowedUID[i]) {
      accessGranted = false;
      break;
    }
  }

  if (accessGranted) {
    unlockDoor("RFID Card");
  } else {
    Serial.println("Access Denied!");
    client.publish(topic_status, "DENIED");
    for(int i = 0; i < 3; i++) {
      digitalWrite(LED_RED, LOW);
      delay(150);
      digitalWrite(LED_RED, HIGH);
      delay(150);
    }
  }
  
  delay(1000);
}