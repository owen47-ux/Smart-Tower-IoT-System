#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --- กำหนดค่า Wi-Fi และ MQTT Broker ---
const char* ssid = "tanapark_104";
const char* password = "00104even";
const char* mqtt_server = "192.168.1.9"; 

WiFiClient espClient;
PubSubClient client(espClient);

// --- กำหนดขาพินสำหรับเซนเซอร์ PIR (Input) ---
#define PIR_1 5  // D1
#define PIR_2 4  // D2
#define PIR_3 14 // D5
#define PIR_4 12 // D6

// --- กำหนดขาพินสำหรับ LED แสดงสถานะ และ Buzzer (Output) ---
#define LED_1 16 // D0
#define LED_2 0  // D3
#define LED_3 13 // D7
#define LED_4 15 // D8
#define BUZZER 2 // D4

// ตัวแปรเก็บสถานะเดิม
int lastStatusPIR1 = LOW;
int lastStatusPIR2 = LOW;
int lastStatusPIR3 = LOW;
int lastStatusPIR4 = LOW;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // ตั้งค่าขา PIR (Input)
  pinMode(PIR_1, INPUT);
  pinMode(PIR_2, INPUT);
  pinMode(PIR_3, INPUT);
  pinMode(PIR_4, INPUT);
  
  // ตั้งค่าขา LED & Buzzer (Output)
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // ปิดไฟและเสียงเริ่มต้น
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, LOW);
  digitalWrite(BUZZER, LOW);

  setup_wifi();
  client.setServer(mqtt_server, 1883);

  Serial.println("--- Perimeter Security Sensor Ready ---");
  Serial.println("Waiting for warm up PIR Sensors (approx 10-60s)...");
  delay(10000); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // อ่านค่าดิจิทัลจากเซนเซอร์แต่ละตัว
  int statusPIR1 = digitalRead(PIR_1);
  int statusPIR2 = digitalRead(PIR_2);
  int statusPIR3 = digitalRead(PIR_3);
  int statusPIR4 = digitalRead(PIR_4);

  // ตรวจสอบและส่งค่าโซน 1
  if (statusPIR1 != lastStatusPIR1) {
    lastStatusPIR1 = statusPIR1;
    if (statusPIR1 == HIGH) {
      digitalWrite(LED_1, HIGH);
      client.publish("security/corner1", "1");
      Serial.println("Corner 1: Motion Detected");
    } else {
      digitalWrite(LED_1, LOW);
      client.publish("security/corner1", "0");
    }
  }

  // ตรวจสอบและส่งค่าโซน 2
  if (statusPIR2 != lastStatusPIR2) {
    lastStatusPIR2 = statusPIR2;
    if (statusPIR2 == HIGH) {
      digitalWrite(LED_2, HIGH);
      client.publish("security/corner2", "1");
      Serial.println("Corner 2: Motion Detected");
    } else {
      digitalWrite(LED_2, LOW);
      client.publish("security/corner2", "0");
    }
  }

  // ตรวจสอบและส่งค่าโซน 3
  if (statusPIR3 != lastStatusPIR3) {
    lastStatusPIR3 = statusPIR3;
    if (statusPIR3 == HIGH) {
      digitalWrite(LED_3, HIGH);
      client.publish("security/corner3", "1");
      Serial.println("Corner 3: Motion Detected");
    } else {
      digitalWrite(LED_3, LOW);
      client.publish("security/corner3", "0");
    }
  }

  // ตรวจสอบและส่งค่าโซน 4
  if (statusPIR4 != lastStatusPIR4) {
    lastStatusPIR4 = statusPIR4;
    if (statusPIR4 == HIGH) {
      digitalWrite(LED_4, HIGH);
      client.publish("security/corner4", "1");
      Serial.println("Corner 4: Motion Detected");
    } else {
      digitalWrite(LED_4, LOW);
      client.publish("security/corner4", "0");
    }
  }

  // ส่งเสียง Buzzer เมื่อมีโซนใดโซนหนึ่งตรวจพบวัตถุ
  if (statusPIR1 == HIGH || statusPIR2 == HIGH || statusPIR3 == HIGH || statusPIR4 == HIGH) {
    digitalWrite(BUZZER, HIGH);
  } else {
    digitalWrite(BUZZER, LOW);
  }

  delay(50);
}