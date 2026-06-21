#include <Adafruit_MPU6050.h>

#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// NHỚ ĐỔI SỐ ĐIỆN THOẠI CỦA BẠN VÀO ĐÂY
const String EMERGENCY_PHONE = "+84";

SoftwareSerial sim800(2, 3);  

#define BUZZER 4
#define BUTTON 5
#define LED_PIN 6

byte updateflag = 0;
float xmove = 0.0, ymove = 0.0, zmove = 0.0, falldetect = 0.0;
float upper_threshold = 30.0, lower_threshold = 4.9;   

boolean freefall_detected = false;
unsigned long freefall_time = 0;
const unsigned long fall_window = 1000; 

boolean impact_detected = false;
unsigned long impact_time;
unsigned long alert_delay = 10000;  
unsigned long last_sample_time = 0; 

// Nhúng 2 file chức năng vào
#include "network.h"
#include "sensor.h"

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  sim800.println("AT"); delay(1000);
  sim800.println("ATE1"); delay(1000);
  sim800.println("AT+CPIN?"); delay(1000);
  sim800.println("AT+CMGF=1"); delay(1000);
  sim800.println("AT+CNMI=1,1,0,0,0"); delay(1000);
  
  while (!Serial) delay(10); 
  if (!mpu.begin()) {
    while (1) delay(10);
  }
}

void loop() {
  // 1. Đọc cảm biến liên tục mỗi 50ms
  if (millis() - last_sample_time >= 50) {
    last_sample_time = millis();
    detectImpact();
  }
  
  // 2. Xử lý khi có tai nạn
  if (updateflag > 0) {
    updateflag = 0;
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER, 1000); 
    impact_detected = true;
    impact_time = millis();
  }
  
  // 3. Thực hiện gọi điện/nhắn tin sau độ trễ 10 giây
  if (impact_detected == true) {
    if (millis() - impact_time >= alert_delay) {
      noTone(BUZZER); 
      makeCall();      // Gọi điện trước
      delay(1000);
      sendAlert();     // Nhắn tin sau
      impact_detected = false;
      impact_time = 0;
      digitalWrite(LED_PIN, LOW);
    }
  }

  // 4. Nút nhấn hủy báo động (Nếu người ngã tự đứng lên được)
  if (digitalRead(BUTTON) == LOW) {
    delay(200); 
    noTone(BUZZER);
    digitalWrite(LED_PIN, LOW);
    impact_detected = false;
    impact_time = 0;
  }

  // 5. Quản lý luồng dữ liệu SIM
  while (sim800.available()) {
    parseData(sim800.readString());
    
  }
  while (Serial.available()) {
    sim800.println(Serial.readString());
  }
}
