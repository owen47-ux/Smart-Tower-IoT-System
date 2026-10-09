// กำหนดขาพินสำหรับเซนเซอร์ PIR (Input)
#define PIR_1 5  // D1
#define PIR_2 4  // D2
#define PIR_3 14 // D5
#define PIR_4 12 // D6

// กำหนดขาพินสำหรับไฟ LED ประจำป้อม (Output)
#define LED_1 0  // D3
#define LED_2 2  // D4
#define LED_3 13 // D7
#define LED_4 15 // D8

// กำหนดขาพินสำหรับลำโพงบัซเซอร์
#define BUZZER_PIN 16 // ย้ายมาใช้ D0 (GPIO 16)

void setup() {
  Serial.begin(115200);
  
  // ตั้งค่าปุ่มรับสัญญาณจากเซนเซอร์ PIR ทั้ง 4 ตัว
  pinMode(PIR_1, INPUT);
  pinMode(PIR_2, INPUT);
  pinMode(PIR_3, INPUT);
  pinMode(PIR_4, INPUT);
  
  // ตั้งค่าขาควบคุมไฟ LED
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  
  // ตั้งค่าขาควบคุมบัซเซอร์
  pinMode(BUZZER_PIN, OUTPUT);
  
  // ทดสอบปิดไฟและบัซเซอร์ตอนเริ่มต้น
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("--- Perimeter Security System Ready ---");
  Serial.println("Waiting for warm up PIR Sensors (approx 10-60s)...");
  delay(10000); // หน่วงเวลาให้เซนเซอร์ปรับค่าแรงดันเริ่มต้นก่อนทำงาน
}

void loop() {
  // อ่านค่าดิจิทัลจากเซนเซอร์แต่ละตัว (HIGH = ตรวจพบการเคลื่อนไหว)
  int statusPIR1 = digitalRead(PIR_1);
  int statusPIR2 = digitalRead(PIR_2);
  int statusPIR3 = digitalRead(PIR_3);
  int statusPIR4 = digitalRead(PIR_4);
  
  bool intrusionDetected = false;

  // ตรวจสอบป้อมที่ 1
  if (statusPIR1 == HIGH) {
    Serial.println("⚠️ ALERT: Intrusion detected at Corner 1!");
    blinkLEDAndSound(LED_1, 200);
    intrusionDetected = true;
  } else {
    digitalWrite(LED_1, LOW);
  }

  // ตรวจสอบป้อมที่ 2
  if (statusPIR2 == HIGH) {
    Serial.println("⚠️ ALERT: Intrusion detected at Corner 2!");
    blinkLEDAndSound(LED_2, 200);
    intrusionDetected = true;
  } else {
    digitalWrite(LED_2, LOW);
  }

  // ตรวจสอบป้อมที่ 3
  if (statusPIR3 == HIGH) {
    Serial.println("⚠️ ALERT: Intrusion detected at Corner 3!");
    blinkLEDAndSound(LED_3, 200);
    intrusionDetected = true;
  } else {
    digitalWrite(LED_3, LOW);
  }

  // ตรวจสอบป้อมที่ 4
  if (statusPIR4 == HIGH) {
    Serial.println("⚠️ ALERT: Intrusion detected at Corner 4!");
    blinkLEDAndSound(LED_4, 200);
    intrusionDetected = true;
  } else {
    digitalWrite(LED_4, LOW);
  }

  // หากไม่มีผู้บุกรุกเลย ให้ปิดเสียงบัซเซอร์
  if (!intrusionDetected) {
    digitalWrite(BUZZER_PIN, LOW);
  }

  delay(100); // หน่วงเวลาสั้นๆ ก่อนเริ่มลูปใหม่เพื่อเสถียรภาพของบอร์ด
}

// ฟังก์ชันสำหรับสั่งให้ไฟกระพริบและบัซเซอร์ส่งเสียงร้องพร้อมกัน
void blinkLEDAndSound(int ledPin, int duration) {
  digitalWrite(ledPin, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(ledPin, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  delay(duration);
}