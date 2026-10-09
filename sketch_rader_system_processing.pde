import processing.serial.*;
import java.awt.event.KeyEvent;
import java.io.IOException;

Serial myPort;
String angle = "";
String distance = "";
String data = "";
int iAngle = 0;
int iDistance = 0;
float pixsDistance;

void setup() {
  size(1200, 700); // ขนาดหน้าจอ
  smooth();
  
  // *** อย่าลืมเปลี่ยน COM5 ให้ตรงกับพอร์ตของคุณ ***
  myPort = new Serial(this, "COM5", 115200);
  myPort.bufferUntil('.');
}

void draw() {
  // สร้างเอฟเฟกต์เงาลากยาวของเส้นสแกนเรดาร์ (Fade Effect ให้ละมุน)
  fill(0, 15); 
  noStroke();
  rect(0, 0, width, height - height * 0.065); 
  
  // วาดองค์ประกอบเรดาร์
  drawRadar(); 
  drawLine();
  drawObject();
  drawText();
}

void serialEvent(Serial myPort) {
  try {
    data = myPort.readStringUntil('.');
    if (data != null) {
      data = trim(data); // ลบช่องว่าง/บรรทัดใหม่ออก
      if (data.length() > 0 && data.endsWith(".")) {
        data = data.substring(0, data.length() - 1); // ตัดจุดออก
        int index1 = data.indexOf(",");
        if (index1 > 0) {
          angle = data.substring(0, index1);
          distance = data.substring(index1 + 1, data.length());
          
          iAngle = int(trim(angle));
          iDistance = int(trim(distance));
        }
      }
    }
  } catch (Exception e) {
    // ป้องกันโปรแกรมแครชกรณีอ่านข้อมูลผิดพลาดช่วงเริ่มต้น
  }
}

void drawRadar() {
  pushMatrix();
  translate(width/2, height - height*0.074);
  noFill();
  strokeWeight(2);
  stroke(98, 245, 31);
  
  float maxRadius = (width - width*0.0625) / 2.0;
  
  // วาดเส้นวงกลมเรดาร์ (4 วง)
  arc(0, 0, (width - width*0.0625), (width - width*0.0625), PI, TWO_PI);
  arc(0, 0, (width - width*0.27), (width - width*0.27), PI, TWO_PI);
  arc(0, 0, (width - width*0.479), (width - width*0.479), PI, TWO_PI);
  arc(0, 0, (width - width*0.687), (width - width*0.687), PI, TWO_PI);
  
  // วาดเส้นรัศมีมุม
  line(-maxRadius, 0, maxRadius, 0);
  line(0, 0, maxRadius * cos(radians(30)), -maxRadius * sin(radians(30)));
  line(0, 0, maxRadius * cos(radians(60)), -maxRadius * sin(radians(60)));
  line(0, 0, maxRadius * cos(radians(90)), -maxRadius * sin(radians(90)));
  line(0, 0, maxRadius * cos(radians(120)), -maxRadius * sin(radians(120)));
  line(0, 0, maxRadius * cos(radians(150)), -maxRadius * sin(radians(150)));
  
  // ตัวเลขบอกมุมรอบขอบวงกลมเรดาร์
  fill(98, 245, 31);
  textSize(18);
  textAlign(CENTER);
  text("30°", (maxRadius + 25) * cos(radians(30)), -(maxRadius + 25) * sin(radians(30)));
  text("60°", (maxRadius + 25) * cos(radians(60)), -(maxRadius + 25) * sin(radians(60)));
  text("90°", (maxRadius + 25) * cos(radians(90)), -(maxRadius + 25) * sin(radians(90)));
  text("120°", (maxRadius + 25) * cos(radians(120)), -(maxRadius + 25) * sin(radians(120)));
  text("150°", (maxRadius + 25) * cos(radians(150)), -(maxRadius + 25) * sin(radians(150)));
  
  popMatrix();
}

void drawObject() {
  pushMatrix();
  translate(width/2, height - height*0.074);
  strokeWeight(9);
  stroke(255, 10, 10); // สีแดงเมื่อเจอวัตถุ
  
  float maxRadius = (width - width*0.0625) / 2.0;
  pixsDistance = iDistance * (maxRadius / 40.0);
  
  // วาดเฉพาะมุมปัจจุบัน เพื่อให้เกิดเอฟเฟกต์จางสแกนสมูทแบบเดิม
  if (iDistance > 0 && iDistance < 40) {
    line(pixsDistance * cos(radians(iAngle)), -pixsDistance * sin(radians(iAngle)),
         maxRadius * cos(radians(iAngle)), -maxRadius * sin(radians(iAngle)));
  }
  popMatrix();
}

void drawLine() {
  pushMatrix();
  strokeWeight(9);
  stroke(30, 250, 60); // สีเขียวสแกน
  translate(width/2, height - height*0.074);
  float maxRadius = (width - width*0.0625) / 2.0;
  
  // วาดเส้นกวาดสแกนตั้งแต่จุดศูนย์กลางถึงขอบเรดาร์
  line(0, 0, maxRadius * cos(radians(iAngle)), -maxRadius * sin(radians(iAngle)));
  popMatrix();
}

void drawText() {
  pushMatrix();
  
  // แถบดำด้านล่างสำหรับแสดงข้อความ
  fill(0);
  noStroke();
  rect(0, height - height*0.065, width, height);
  
  fill(98, 245, 31);
  textSize(20);
  textAlign(LEFT);
  
  // แสดงสเกลระยะทางบนเรดาร์
  text("10cm", width - width*0.385, height - height*0.083);
  text("20cm", width - width*0.281, height - height*0.083);
  text("30cm", width - width*0.177, height - height*0.083);
  text("40cm", width - width*0.073, height - height*0.083);
  
  // แสดงค่าสถานะปัจจุบันที่ด้านล่าง
  textSize(25);
  text("Object: " + (iDistance < 40 && iDistance > 0 ? "In Range" : "Out of Range"), width * 0.05, height - height * 0.025);
  text("Angle: " + iAngle + "°", width * 0.48, height - height * 0.025);
  text("Distance: " + (iDistance < 40 && iDistance > 0 ? iDistance + " cm" : "---"), width * 0.72, height - height * 0.025);
  
  popMatrix();
}
