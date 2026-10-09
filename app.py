import os
import numpy as np
import cv2
import face_recognition
from flask import Flask, request, jsonify

app = Flask(__name__)

# กำหนดโฟลเดอร์สำหรับเก็บรูปใบหน้าคนที่ได้รับอนุญาต (เช่น owen.jpg)
KNOWN_FACES_DIR = "known_faces"
known_face_encodings = []
known_face_names = []

def load_known_faces():
    """ฟังก์ชันโหลดและสกัดเวกเตอร์ใบหน้า (Face Encodings) จากโฟลเดอร์ known_faces"""
    global known_face_encodings, known_face_names
    known_face_encodings = []
    known_face_names = []
    
    if not os.path.exists(KNOWN_FACES_DIR):
        os.makedirs(KNOWN_FACES_DIR)
        print(f"Created directory: {KNOWN_FACES_DIR}")
        return

    for filename in os.listdir(KNOWN_FACES_DIR):
        if filename.lower().endswith(('.png', '.jpg', '.jpeg')):
            name = os.path.splitext(filename)[0]  # ดึงชื่อคนจากชื่อไฟล์ เช่น owen.jpg -> owen
            filepath = os.path.join(KNOWN_FACES_DIR, filename)
            
            image = face_recognition.load_image_file(filepath)
            encodings = face_recognition.face_encodings(image)
            
            if len(encodings) > 0:
                known_face_encodings.append(encodings[0])
                known_face_names.append(name)
                print(f"[AI Server] Loaded known face: {name}")

# โหลดข้อมูลใบหน้าทันทีเมื่อเริ่มรันโปรแกรม
load_known_faces()

@app.route('/scan', methods=['POST'])
def scan_face():
    """API Endpoint รับรูปภาพจาก Node-RED เพื่อวิเคราะห์ใบหน้า"""
    if 'file' not in request.files and 'image' not in request.files:
        return jsonify({
            "is_known": False,
            "name": "No Image Provided",
            "status": "danger"
        }), 400

    file = request.files.get('file') or request.files.get('image')
    
    # แปลงไฟล์ภาพที่รับมาเป็นอาร์เรย์รูปภาพสำหรับ OpenCV / face_recognition
    file_bytes = np.frombuffer(file.read(), np.uint8)
    img = cv2.imdecode(file_bytes, cv2.IMREAD_COLOR)
    rgb_img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    # ตรวจจับตำแหน่งและคำนวณสกัดเวกเตอร์ใบหน้า (128-d Vector)
    face_locations = face_recognition.face_locations(rgb_img)
    face_encodings = face_recognition.face_encodings(rgb_img, face_locations)

    # กรณีที่ 1: ไม่พบใบหน้ามนุษย์ในรูปเลย
    if len(face_encodings) == 0:
        return jsonify({
            "is_known": False,
            "name": "No Face Detected",
            "status": "danger"
        })

    # กรณีที่ 2: พบใบหน้า ให้นำไปเปรียบเทียบกับใบหน้าในโฟลเดอร์ known_faces
    for face_encoding in face_encodings:
        if len(known_face_encodings) == 0:
            break

        # คำนวณหาระยะห่างทางเวกเตอร์ (Face Distance)
        face_distances = face_recognition.face_distance(known_face_encodings, face_encoding)
        best_match_index = np.argmin(face_distances)

        # กำหนด Threshold (หากค่าระยะห่างน้อยกว่า 0.55 แสดงว่าเป็นบุคคลเดียวกัน)
        if face_distances[best_match_index] < 0.55:
            name = known_face_names[best_match_index]
            return jsonify({
                "is_known": True,
                "name": name,
                "status": "safe"
            })

    # กรณีที่ 3: สแกนเจอใบหน้า แต่ไม่ตรงกับใครในระบบ (คนแปลกหน้า)
    return jsonify({
        "is_known": False,
        "name": "Unknown Person",
        "status": "danger"
    })

@app.route('/reload_faces', methods=['GET'])
def reload_faces():
    """API สำรองสำหรับสั่งให้เซิร์ฟเวอร์โหลดรูปใน known_faces ใหม่โดยไม่ต้องรีสตาร์ต"""
    load_known_faces()
    return jsonify({"status": "success", "total_known": len(known_face_names)})

if __name__ == '__main__':
    # รันเซิร์ฟเวอร์ที่ Port 5000
    app.run(host='0.0.0.0', port=5000)