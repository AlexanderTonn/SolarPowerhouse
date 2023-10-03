#include "RTC.h"
#include "controlLogic.h"


// ##################
// DS3231: RTC MODE
// ##################
void rtcInit() {
  // Checking OscillatorStopFlag
  Serial.println("Start Checking OSF");
  //Wire.beginTransmission(0x68);
  //Wire.write(0x0F);  //status register, which contains OSF Flag on 7th position
  //Wire.endTransmission();
  Wire.requestFrom(0x68, 1);  //Request for getting content of status register

  byte by = Wire.read();
  if ((by & 0x80) != 0x80) {
    Serial.println("RTC running OSF=0");
  } else {
    Serial.println("RTC not running OSF=1");
  }
}

// Check whether day has changed;
// -> if yes, return true
// -> consider the option of Page 2
auto rtcCheckDayChange(bool xSettDayChangeActive) -> bool{
  if(!xSettDayChangeActive)
  {
    return false;
  }
  // if xSettDayChangeActive is true, check whether day has changed
  else
  {
    byte byDayActual = rtcDs3231.getDate();
    if (byDayActual != byDateOld) {
      byDateOld = byDayActual;
      return true;
    } else
      return false;
  }
}