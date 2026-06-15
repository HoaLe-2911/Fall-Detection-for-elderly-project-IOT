#include <Arduino.h>
#include <AltSoftSerial.h>
#include <TinyGPS++.h>
#include <Adafruit_MPU6050.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_temp, *mpu_accel, *mpu_gyro;

const String EMERGENCY_PHONE = "0356181025";

SoftwareSerial sim800(2, 3);  
AltSoftSerial neogps; // Fixed typo from original code
TinyGPSPlus gps;

String sms_status, sender_number, received_date, msg;
String latitude, longitude;

#define BUZZER 4
#define BUTTON 5

byte updateflag = 0;

// Upgraded to float for higher precision (Fixes the "Total: 9" bug)
float xmove = 0.0;
float ymove = 0.0;
float zmove = 0.0;

int vibration = 1, devibrate = 5;
float falldetect = 0.0;
float sensitivity = 15.65; // Adjust this sensitivity threshold during testing
boolean impact_detected = false;

unsigned long time1;
unsigned long impact_time;
unsigned long alert_delay = 10000;  // Delay before sending SMS/Call (10 seconds)

// Function prototypes declaration
void detectImpact();
void getGps();
void sendAlert();
void makeCall();
void parseData(String buff);

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  neogps.begin(9600);
  
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  
  sms_status = "";
  sender_number = "";
  received_date = "";
  msg = "";
  
  // Initialize SIM module
  sim800.println("AT");
  delay(1000);
  sim800.println("ATE1");
  delay(1000);
  sim800.println("AT+CPIN?");
  delay(1000);
  sim800.println("AT+CMGF=1");
  delay(1000);
  sim800.println("AT+CNMI=1,1,0,0,0"); 
  delay(1000);
  
  time1 = micros();
 
  while (!Serial) delay(10); 

  Serial.println("Initializing MPU6050...");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip!");
    while (1) {
      delay(10);
    }
  }

  Serial.println("MPU6050 Found!");
  mpu_accel = mpu.getAccelerometerSensor();
}

void loop() {
  // Read sensor data
  if (micros() - time1 > 1999) detectImpact();
  
  // Handle fall detection event
  if (updateflag > 0) {
    updateflag = 0;
    Serial.println("!!! FALL DETECTED !!!");
    Serial.print("Magnitude: ");
    Serial.println(falldetect);

    getGps();
    tone(BUZZER, 1000); // Sound the active/passive buzzer
    impact_detected = true;
    impact_time = millis();
  }
  
  // Timeout handling for alert protocol
  if (impact_detected == true) {
    if (millis() - impact_time >= alert_delay) {
      noTone(BUZZER); // Turn off buzzer before making the call
      makeCall();
      delay(1000);
      sendAlert();
      impact_detected = false;
      impact_time = 0;
    }
  }

  // Manual cancellation via push button
  if (digitalRead(BUTTON) == LOW) {
    delay(200); // Debounce
    noTone(BUZZER);
    impact_detected = false;
    impact_time = 0;
    Serial.println("Alarm manually cancelled by user.");
  }

  // Listen for incoming SIM messages
  while (sim800.available()) {
    parseData(sim800.readString());
  }
  while (Serial.available()) {
    sim800.println(Serial.readString());
  }
}

void detectImpact() {
  time1 = micros();  // Resets time value
  sensors_event_t accel;
  mpu_accel->getEvent(&accel);
  
  Serial.print("X: "); Serial.print(accel.acceleration.x);
  Serial.print(" \tY: "); Serial.print(accel.acceleration.y);
  Serial.print(" \tZ: "); Serial.print(accel.acceleration.z);
  Serial.println(" m/s^2 ");

  delay(200);

  vibration--;
  if (vibration < 0) vibration = 0;
  if (vibration > 0) return;
  
  xmove = accel.acceleration.x;
  ymove = accel.acceleration.y;
  zmove = accel.acceleration.z;

  // Calculate total acceleration vector
  falldetect = sqrt(xmove * xmove + ymove * ymove + zmove * zmove);
  Serial.print(" | Total: "); Serial.println(falldetect);
  
  if (falldetect >= sensitivity) {
    updateflag = 1;
    vibration = devibrate;
  } else {
    falldetect = 0;
  }
}

void parseData(String buff) {
  Serial.println(buff);
  unsigned int index;
 
  index = buff.indexOf("\r");
  buff.remove(0, index + 2);
  buff.trim();
  
  if (buff != "OK") {
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();

    buff.remove(0, index + 2);
    if (cmd == "+CMTI") {
      index = buff.indexOf(",");
      String temp = buff.substring(index + 1, buff.length());
      temp = "AT+CMGR=" + temp + "\r";
      sim800.println(temp);
    }
    else if (cmd == "+CMGR") {
      if (buff.indexOf(EMERGENCY_PHONE) > 1) {
        buff.toLowerCase();
        if (buff.indexOf("get gps") > 1) {
          getGps();
          String sms_data = "GPS Location Data\r";
          sms_data += "http://maps.google.com/maps?q=";
          sms_data += latitude + "," + longitude;
          // sendSms(sms_data);
        }
      }
    }
  }
}

void getGps() {
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;) {
    while (neogps.available()) {
      if (gps.encode(neogps.read())) {
        newData = true;
        break;
      }
    }
  }

  if (newData) {
    latitude = String(gps.location.lat(), 6);
    longitude = String(gps.location.lng(), 6);
    newData = false;
  } else {
    Serial.println("No GPS data available.");
    latitude = "0.000000";
    longitude = "0.000000";
  }

  Serial.print("Latitude = "); Serial.println(latitude);
  Serial.print("Longitude = "); Serial.println(longitude);
}

void sendAlert() {
  String sms_data = "EMERGENCY: FALL DETECTED!\r";
  sms_data += "Location link:\r";
  sms_data += "http://maps.google.com/maps?q=";
  sms_data += latitude + "," + longitude;

  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\"" + EMERGENCY_PHONE + "\"\r");
  delay(1000);
  sim800.print(sms_data);
  delay(100);
  sim800.write(0x1A); // ASCII code for Ctrl+Z to send SMS
  delay(1000);
  Serial.println("Emergency SMS Sent Successfully.");
}

void makeCall() {
  Serial.println("Dialing emergency contact...");
  sim800.println("ATD" + EMERGENCY_PHONE + ";");
  delay(10000);  // Ring for 10 seconds
  sim800.println("ATH"); // Hang up command
  delay(1000); 
}