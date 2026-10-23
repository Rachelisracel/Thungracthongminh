#include <Arduino.h>
#include <ESP32Servo.h>

// Khai báo chân
const int IR_SENSOR_PIN = 19;  // Chân nối với nút bấm mô phỏng FC-51 Nắp
const int SERVO_PIN = 13;     // Chân nối với tín hiệu Servo

Servo lidServo;
unsigned long lastDetectedTime = 0; // Lưu thời điểm cuối cùng nhìn thấy tay
bool isOpen = false;                // Trạng thái của nắp

void setup() {
  Serial.begin(115200);
  
  // Dùng INPUT_PULLUP để giữ tín hiệu nút bấm luôn ổn định
  pinMode(IR_SENSOR_PIN, INPUT_PULLUP); 
  
  lidServo.attach(SERVO_PIN);
  lidServo.write(0); // Khởi động ở trạng thái đóng nắp (0 độ)
  
  Serial.println("He thong da khoi dong!");
}

void loop() {
  int sensorState = digitalRead(IR_SENSOR_PIN);

  // Khi nhấn nút trong Wokwi, tín hiệu sẽ là LOW (0)
  if (sensorState == LOW) { 
    if (!isOpen) {
      lidServo.write(90); // Mở nắp 90 độ
      isOpen = true;
      Serial.println("Phát hiện tay -> Mở nắp");
    }
    // Cập nhật thời gian liên tục chừng nào vẫn đang giữ chuột (tay chưa rút ra)
    lastDetectedTime = millis(); 
  } 
  else {
    // Nhả chuột ra (sensorState == HIGH), bắt đầu đếm 3 giây
    if (isOpen && (millis() - lastDetectedTime >= 3000)) {
      lidServo.write(0);  // Đóng nắp
      isOpen = false;
      Serial.println("Đã qua 3s không có tay -> Đóng nắp");
    }
  }
  
  delay(50); // Nghỉ ngắn để ổn định hệ thống
}