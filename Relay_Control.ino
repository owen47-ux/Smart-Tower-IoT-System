#include <WiFi.h>
#include <PubSubClient.h>

// --- ตั้งค่า Wi-Fi & MQTT Server ---
const char* ssid = "tanapark_104";
const char* password = "00104even";
const char* mqtt_server = "192.168.1.9"; // IP เครื่อง Node-RED / MQTT Broker
const int   mqtt_port   = 1883;                   // พอร์ตมาตรฐานของ MQTT

// ==================== กำหนดขา GPIO สำหรับรีเลย์ทั้ง 10 ช่อง ====================
const int TOTAL_RELAYS = 10;
const int relayPins[TOTAL_RELAYS] = {
  16, 17, 18, 19,  // โมดูล 4-CH ตัวที่ 1 (Relay 1 - 4)
  32, 33, 25, 26,  // โมดูล 4-CH ตัวที่ 2 (Relay 5 - 8)
  27, 14           // โมดูล 2-CH (Relay 9 - 10)
};

WiFiClient espClient;
PubSubClient client(espClient);

// ฟังก์ชันสำหรับรับข้อความจาก Node-RED (MQTT Callback)
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Topic received: ");
  Serial.print(topic);
  Serial.print(" | Payload: ");
  Serial.println(message);

  // ตรวจสอบว่าสั่งงานมาจาก Topic ช่องไหน (เช่น home/relay/1/set ถึง home/relay/10/set)
  for (int i = 0; i < TOTAL_RELAYS; i++) {
    String targetTopic = "home/relay/" + String(i + 1) + "/set";
    
    if (String(topic) == targetTopic) {
      if (message == "1" || message == "ON" || message == "on") {
        digitalWrite(relayPins[i], HIGH); // สั่งรีเลย์เปิด (Active HIGH)
        Serial.printf("Relay %d [GPIO %d] -> ON\n", i + 1, relayPins[i]);
      } 
      else if (message == "0" || message == "OFF" || message == "off") {
        digitalWrite(relayPins[i], LOW);  // สั่งรีเลย์ปิด
        Serial.printf("Relay %d [GPIO %d] -> OFF\n", i + 1, relayPins[i]);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  // ตั้งค่าขา GPIO ทุกช่องให้เป็น OUTPUT และเริ่มต้นที่ LOW (ปิดไฟทั้งหมดตอนเปิดเครื่อง)
  for (int i = 0; i < TOTAL_RELAYS; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }

  // เชื่อมต่อ Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ตั้งค่า MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT Broker...");
    String clientId = "ESP32RelayController-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println(" Connected!");
      // Subscribe เพื่อรับคำสั่งควบคุมรีเลย์ทั้ง 10 ช่อง
      client.subscribe("home/relay/+/set");
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}