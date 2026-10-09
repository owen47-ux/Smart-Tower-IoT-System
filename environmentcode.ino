#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ==================== 1. ตั้งค่า Wi-Fi & MQTT Broker ====================
const char* ssid        = "tanapark_104";       // ใส่ชื่อ Wi-Fi ของคุณ
const char* password    = "00104even";   // ใส่รหัสผ่าน Wi-Fi
const char* mqtt_server = "192.168.1.9";        // IP Address เครื่องที่ลง Node-RED
const int   mqtt_port   = 1883;

// ==================== 2. กำหนดขาอุปกรณ์ ====================
// รีเลย์ 4 ช่อง
const int RELAY_PINS[4] = {26, 25, 33, 32}; 

// เซ็นเซอร์ Digital
#define DHT11_PIN       16
#define DHT22_PIN       17
#define DHTTYPE11       DHT11
#define DHTTYPE22       DHT22

// เซ็นเซอร์ Analog (ADC1 Safe Pins)
#define MQ3_PIN         34
#define SOIL_PIN        35
#define LDR_PIN         36

// อุปกรณ์แจ้งเตือน
#define BUZZER_PIN      18
#define RGB_RED_PIN     19
#define RGB_GREEN_PIN   2   // เปลี่ยนเป็น GPIO 2
#define RGB_BLUE_PIN    15  // เปลี่ยนเป็น GPIO 15

// ==================== 3. ตั้งค่าการทำงาน & เกณฑ์ (Threshold) ====================
const float TEMP_COOLING_THRESHOLD = 30.0; // อุณหภูมิ DHT22 เกิน 30°C สั่งเปิดรีเลย์ 4 ตัว
const int   ALCOHOL_THRESHOLD      = 2000; // ค่าแอลกอฮอล์ MQ-3 (0-4095) เกินค่านี้จะแจ้งเตือน

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht11(DHT11_PIN, DHTTYPE11);
DHT dht22(DHT22_PIN, DHTTYPE22);

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastSensorRead = 0;
unsigned long lastBlink      = 0;
bool alarmBlinkState         = false;
bool isAlcoholAlarm          = false;

void setRGB(bool r, bool g, bool b) {
  digitalWrite(RGB_RED_PIN, r ? HIGH : LOW);
  digitalWrite(RGB_GREEN_PIN, g ? HIGH : LOW);
  digitalWrite(RGB_BLUE_PIN, b ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], LOW);
  }

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  setRGB(0, 0, 0);

  Wire.begin(21, 22);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" System Starting ");
  
  dht11.begin();
  dht22.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  client.setServer(mqtt_server, mqtt_port);
  lcd.clear();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32_MultiSensor_";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println(" Connected!");
    } else {
      delay(3000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();

  if (currentMillis - lastSensorRead >= 2000) {
    lastSensorRead = currentMillis;

    float h11 = dht11.readHumidity();
    float t11 = dht11.readTemperature();
    float h22 = dht22.readHumidity();
    float t22 = dht22.readTemperature();

    int mq3Val  = analogRead(MQ3_PIN);
    int soilVal = analogRead(SOIL_PIN);
    int ldrVal  = analogRead(LDR_PIN);

    if (isnan(h11) || isnan(t11)) { h11 = 0; t11 = 0; }
    if (isnan(h22) || isnan(t22)) { h22 = 0; t22 = 0; }

    // ระบายความร้อนผ่านพัดลม 12V เมื่ออุณหภูมิเกินกำหนด
    if (t22 >= TEMP_COOLING_THRESHOLD) {
      for (int i = 0; i < 4; i++) digitalWrite(RELAY_PINS[i], HIGH);
      client.publish("home/relay/status", "ON");
    } else {
      for (int i = 0; i < 4; i++) digitalWrite(RELAY_PINS[i], LOW);
      client.publish("home/relay/status", "OFF");
    }

    // ระบบตรวจจับแอลกอฮอล์
    if (mq3Val >= ALCOHOL_THRESHOLD) {
      isAlcoholAlarm = true;
    } else {
      isAlcoholAlarm = false;
      digitalWrite(BUZZER_PIN, LOW);
      setRGB(0, 1, 0); // ไฟสีเขียวสว่างสภาวะปกติ (GPIO 2)
    }

    lcd.clear();
    if (isAlcoholAlarm) {
      lcd.setCursor(0, 0);
      lcd.print("! WARNING ALCOHOL!");
      lcd.setCursor(0, 1);
      lcd.print("Level: ");
      lcd.print(mq3Val);
    } else {
      lcd.setCursor(0, 0);
      lcd.print("T22:");
      lcd.print(t22, 1);
      lcd.print("C H22:");
      lcd.print(h22, 0);
      lcd.print("%");
      
      lcd.setCursor(0, 1);
      lcd.print("Alc:");
      lcd.print(mq3Val);
      lcd.print(" Soil:");
      lcd.print(map(soilVal, 4095, 0, 0, 100));
    }

    client.publish("home/sensor/dht11_temp", String(t11).c_str());
    client.publish("home/sensor/dht11_hum", String(h11).c_str());
    client.publish("home/sensor/dht22_temp", String(t22).c_str());
    client.publish("home/sensor/dht22_hum", String(h22).c_str());
    client.publish("home/sensor/alcohol", String(mq3Val).c_str());
    client.publish("home/sensor/soil", String(soilVal).c_str());
    client.publish("home/sensor/light", String(ldrVal).c_str());
  }

  if (isAlcoholAlarm) {
    if (currentMillis - lastBlink >= 300) {
      lastBlink = currentMillis;
      alarmBlinkState = !alarmBlinkState;

      if (alarmBlinkState) {
        setRGB(1, 0, 0);               // ไฟสีแดงติด (GPIO 19)
        digitalWrite(BUZZER_PIN, HIGH);
      } else {
        setRGB(0, 0, 0);               // ปิดไฟ
        digitalWrite(BUZZER_PIN, LOW);
      }
    }
  }
}