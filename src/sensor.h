void detectImpact() {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);
  
  xmove = accel.acceleration.x;
  ymove = accel.acceleration.y;
  zmove = accel.acceleration.z;

  falldetect = sqrt(xmove * xmove + ymove * ymove + zmove * zmove);

  /
  Serial.print("Acceleration ->  X: "); Serial.print(xmove, 2);
  Serial.print(" | Y: ");          Serial.print(ymove, 2);
  Serial.print(" | Z: ");          Serial.print(zmove, 2);
  Serial.print("  ==>  SUM: ");   Serial.println(falldetect, 2);
  // ----------------------------------------------

 
  if (falldetect <= lower_threshold) {
    freefall_detected = true;
    freefall_time = millis();
  }


  if (falldetect >= upper_threshold) {
    if (freefall_detected == true && (millis() - freefall_time <= fall_window)) {
      updateflag = 1;
      freefall_detected = false;
    }
  }

  if (freefall_detected == true && (millis() - freefall_time > fall_window)) {
     freefall_detected = false;
  }
}