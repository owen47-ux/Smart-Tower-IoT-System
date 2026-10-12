#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --- ตั้งค่า Wi-Fi ---
const char* ssid = "";      // ชื่อ Wi-Fi ของคุณ
const char* password = "";   // รหัสผ่าน Wi-Fi

// --- ตั้งค่า MQTT ---
const char* mqtt_server = ""; // IP เครื่องเซิร์ฟเวอร์ Node-RED ของคุณ
const int mqtt_port = 1883;
const char* mqtt_topic = "security/ir_trigger";

// --- กำหนดพิน ---
#define IR_PIN     D5  // พินรับสัญญาณจาก IR Sensor
#define LED_RED    D2  // พิน LED สีแดง
#define LED_GREEN  D3  // พิน LED สีเขียว
#define BUZZER_PIN D6  // พิน Buzzer

WiFiClient espClient;
PubSubClient client(espClient);

bool objectDetected = false;

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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-IR-";
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
  
  pinMode(IR_PIN, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // สถานะเริ่มต้น: ไฟแดงติด, ไฟเขียวดับ, บัซเซอร์เงียบ
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // อ่านค่าจาก IR Sensor (โมดูลส่วนใหญ่จะส่งค่า LOW เมื่อตรวจเจอบางอย่างบัง)
  int irVal = digitalRead(IR_PIN);

  if (irVal == LOW) { 
    if (!objectDetected) { // ทำงานเมื่อมีวัตถุเข้ามาตัดผ่านครั้งแรก
      objectDetected = true;
      Serial.println("!!! Object Detected !!!");

      // สลับไฟสถานะ: แดงดับ / เขียวติด
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, HIGH);

      // เสียงเตือนสั้นๆ (Beep)
      digitalWrite(BUZZER_PIN, HIGH);
      delay(150);
      digitalWrite(BUZZER_PIN, LOW);

      // ส่งสัญญาณ MQTT ไปบอก Node-RED ว่ามีคนผ่าน
      client.publish(mqtt_topic, "TRIGGER");
    }
  } else {
    if (objectDetected) { // คืนค่าเมื่อไม่มีวัตถุบังแล้ว
      objectDetected = false;
      Serial.println("Clear - Standby Mode");

      // สลับไฟกลับ: แดงติด / เขียวดับ
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  delay(50); // พักการสแกนเล็กน้อย
}
