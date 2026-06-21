#pragma once

void sendAlert() {
  String sms_data = "EMERGENCY: FALL DETECTED!\r";
  sms_data += "Vui long kiem tra nguoi than ngay lap tuc!"; 

  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\"" + EMERGENCY_PHONE + "\"\r");
  delay(1000);
  sim800.print(sms_data);
  delay(100);
  sim800.write(0x1A); 
  delay(1000);
}

void makeCall() {
  sim800.println("ATD" + EMERGENCY_PHONE + ";");
  delay(10000);  
  sim800.println("ATH"); // Cúp máy
  delay(1000); 
}

void parseData(String buff) {
  int index;
 
  index = buff.indexOf("\r");
  if (index != -1) {
    buff.remove(0, index + 2);
    buff.trim();
  }
}