#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ==================== 1. ตั้งค่า Wi-Fi & MQTT Broker ====================
const char* ssid         = "tanapark_104";      // ชื่อ Wi-Fi
const char* password     = "00104even";          // รหัสผ่าน Wi-Fi
const char* mqtt_server  = "192.168.1.9";        // IP Address เครื่องที่ลง Node-RED
const int   mqtt_port    = 1883;

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
#define RGB_GREEN_PIN   2   // GPIO 2
#define RGB_BLUE_PIN    15  // GPIO 15

// ตั้งค่าชนิดของ RGB LED (Common Anode = Active LOW)
#define RGB_COMMON_ANODE true 

// กำหนดสถานะตรรกะรีเลย์ (Active HIGH)
#define RELAY_ON  HIGH
#define RELAY_OFF LOW

// ==================== 3. ตั้งค่าการทำงาน & เกณฑ์ (Threshold) ====================
float tempThreshold         = 30.0; // ค่าเกณฑ์อุณหภูมิสำหรับ Auto Mode (°C)
int   alcoholThresholdPercent = 40; // ค่าเกณฑ์แก๊ส MQ-3 สำหรับทริกสัญญาณเตือน (%)

bool isAutoMode             = true; // true = AUTO, false = MANUAL
bool relayState[4]          = {false, false, false, false};

DHT dht11(DHT11_PIN, DHTTYPE11);
DHT dht22(DHT22_PIN, DHTTYPE22);

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastSensorRead = 0;
unsigned long lastBlink      = 0;
bool alarmBlinkState         = false;
bool isAlcoholAlarm          = false;

// ฟังก์ชันควบคุมไฟ RGB LED
void setRGB(bool r, bool g, bool b) {
  if (RGB_COMMON_ANODE) {
    digitalWrite(RGB_RED_PIN, r ? LOW : HIGH);
    digitalWrite(RGB_GREEN_PIN, g ? LOW : HIGH);
    digitalWrite(RGB_BLUE_PIN, b ? LOW : HIGH);
  } else {
    digitalWrite(RGB_RED_PIN, r ? HIGH : LOW);
    digitalWrite(RGB_GREEN_PIN, g ? HIGH : LOW);
    digitalWrite(RGB_BLUE_PIN, b ? HIGH : LOW);
  }
}

void setRelay(int index, bool state) {
  if (index >= 0 && index < 4) {
    relayState[index] = state;
    digitalWrite(RELAY_PINS[index], state ? RELAY_ON : RELAY_OFF);
    String topic = "home/relay" + String(index + 1) + "/status";
    client.publish(topic.c_str(), state ? "ON" : "OFF");
  }
}

void setAllRelays(bool state) {
  for (int i = 0; i < 4; i++) {
    setRelay(i, state);
  }
  client.publish("home/relay/status", state ? "ON" : "OFF");
}

// ==================== 4. MQTT Callback Function ====================
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  String strTopic = String(topic);

  // 1. สลับโหมด AUTO / MANUAL
  if (strTopic == "home/mode/set") {
    if (message == "AUTO" || message == "auto" || message == "1") {
      isAutoMode = true;
      client.publish("home/mode/status", "AUTO");
    } else if (message == "MANUAL" || message == "manual" || message == "0") {
      isAutoMode = false;
      client.publish("home/mode/status", "MANUAL");
    }
  }

  // 2. รับค่าจาก Slider เกณฑ์อุณหภูมิ AUTO
  if (strTopic == "home/threshold/temp/set") {
    float val = message.toFloat();
    if (val > 0) {
      tempThreshold = val;
      client.publish("home/threshold/temp/status", String(tempThreshold, 1).c_str());
    }
  }

  // 3. รับค่าจาก Slider เกณฑ์แจ้งเตือนแก๊ส MQ-3 (%)
  if (strTopic == "home/threshold/gas/set") {
    int val = message.toInt();
    if (val >= 0 && val <= 100) {
      alcoholThresholdPercent = val;
      client.publish("home/threshold/gas/status", String(alcoholThresholdPercent).c_str());
    }
  }

  // 4. ควบคุมรีเลย์รวม 4 ช่อง (ทำงานเฉพาะ Manual Mode)
  if (strTopic == "home/relay/set") {
    if (!isAutoMode) {
      bool turnOn = (message == "ON" || message == "1");
      setAllRelays(turnOn);
    }
  }

  // 5. ควบคุมรีเลย์แยกช่อง 1 - 4 (ทำงานเฉพาะ Manual Mode)
  for (int i = 0; i < 4; i++) {
    String relaySetTopic = "home/relay" + String(i + 1) + "/set";
    if (strTopic == relaySetTopic) {
      if (!isAutoMode) {
        bool turnOn = (message == "ON" || message == "1");
        setRelay(i, turnOn);
      }
      break;
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32_MultiSensor_";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println(" Connected!");
      
      client.subscribe("home/mode/set");
      client.subscribe("home/threshold/temp/set");
      client.subscribe("home/threshold/gas/set");
      client.subscribe("home/relay/set");
      client.subscribe("home/relay1/set");
      client.subscribe("home/relay2/set");
      client.subscribe("home/relay3/set");
      client.subscribe("home/relay4/set");

      client.publish("home/mode/status", isAutoMode ? "AUTO" : "MANUAL");
      client.publish("home/threshold/temp/status", String(tempThreshold, 1).c_str());
      client.publish("home/threshold/gas/status", String(alcoholThresholdPercent).c_str());
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Try again in 3 seconds");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
  }

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // สถานะเริ่มต้นสีเขียวสว่าง (ภาวะปกติ)
  setRGB(0, 1, 0); 

  dht11.begin();
  dht22.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();

  // อ่านค่าเซ็นเซอร์ทุกๆ 2 วินาที
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

    // ==================== แปลงค่า Analog เป็นเปอร์เซ็นต์ (0 - 100%) ====================
    int mq3Percent  = map(mq3Val, 0, 4095, 0, 100);
    
    // ความชื้นในดิน: แห้ง/ลอยอากาศ (4095) = 0%, กำแน่น/ชุ่มน้ำ (0) = 100%
    int soilPercent = map(soilVal, 4095, 0, 0, 100); 

    // ความเข้มแสง LDR: มืดสนิท (4095) = 0%, สว่างมาก (0) = 100%
    int ldrPercent  = map(ldrVal, 4095, 0, 0, 100);  

    mq3Percent  = constrain(mq3Percent, 0, 100);
    soilPercent = constrain(soilPercent, 0, 100);
    ldrPercent  = constrain(ldrPercent, 0, 100);

    // --- ระบบ AUTO: ทำงานตามค่าอุณหภูมิ DHT11 ---
    if (isAutoMode) {
      if (t11 >= tempThreshold) {
        if (!relayState[0]) setAllRelays(true);
      } else {
        if (relayState[0]) setAllRelays(false);
      }
    }

    // --- ระบบตรวจจับแก๊สรั่ว MQ-3 ---
    if (mq3Percent >= alcoholThresholdPercent) {
      if (!isAlcoholAlarm) {
        isAlcoholAlarm = true;
        client.publish("home/alarm/gas", "ALARM");
      }
    } else {
      if (isAlcoholAlarm) {
        isAlcoholAlarm = false;
        digitalWrite(BUZZER_PIN, LOW);
        setRGB(0, 1, 0);
        client.publish("home/alarm/gas", "NORMAL");
      }
    }

    // --- ส่งค่าเซ็นเซอร์ขึ้น MQTT ---
    client.publish("home/sensor/dht11_temp", String(t11, 1).c_str());
    client.publish("home/sensor/dht11_hum", String(h11, 1).c_str());
    client.publish("home/sensor/dht22_temp", String(t22, 1).c_str());
    client.publish("home/sensor/dht22_hum", String(h22, 1).c_str());
    client.publish("home/sensor/alcohol", String(mq3Percent).c_str()); 
    client.publish("home/sensor/soil", String(soilPercent).c_str());   
    client.publish("home/sensor/light", String(ldrPercent).c_str());   
  }

  // --- สัญญาณเตือนแก๊สรั่ว (กระพริบไฟ RGB สีแดง + Buzzer) ---
  if (isAlcoholAlarm) {
    if (currentMillis - lastBlink >= 300) {
      lastBlink = currentMillis;
      alarmBlinkState = !alarmBlinkState;

      if (alarmBlinkState) {
        setRGB(1, 0, 0);
        digitalWrite(BUZZER_PIN, HIGH);
      } else {
        setRGB(0, 0, 0);
        digitalWrite(BUZZER_PIN, LOW);
      }
    }
  }
}