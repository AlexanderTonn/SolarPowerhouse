#include "RTC.h"
#include "controlLogic.h"


// ##################
// DS3231: RTC MODE
// ##################
void rtcInit() {
  // Checking OscillatorStopFlag
  Serial.println("Start Checking OSF");
  Wire.beginTransmission(0x68);
  Wire.write(0x0F);  //status register, which contains OSF Flag on 7th position
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);  //Request for getting content of status register

  byte by = Wire.read();
  if ((by & 0x80) != 0x80) {
    Serial.println("RTC running OSF=0");
  } else {
    Serial.println("RTC not running OSF=1");
  }
}
void ds3231SetupHandler() {

  Serial.println("######### RTC Time Init! #########");

  bool xSetupActive = false;  // can getting false at iCase = 10
  // detecting if user want change more options
  do {
    if (xSetupActive == true)
      Serial.begin(uiBaudrate);
    // Wait until Input

    Serial.println("usage: <option>,<value>");
    Serial.println("Available Options - please consider the format!");
    Serial.println("1 = Year (YY)");
    Serial.println("2 = Month (MM)");
    Serial.println("3 = Day (DD)");
    Serial.println("4 = Day of Week (d)");
    Serial.println("5 = Hour (HH)");
    Serial.println("6 = Minutes (MM)");
    Serial.println("7 = Seconds (SS)");
    Serial.println("8 = Clockmode (1=12h / 0=24h)");
    Serial.println("9 = Cancel ");

    while (Serial.available() <= 0)
      ;

    // Read Serial for Getting input Option --> only one character
    if (Serial.available() > 0) {
      String sTerminalInput = Serial.readStringUntil('\n');
      char cTerminalInput[6];
      sTerminalInput.toCharArray(cTerminalInput, sizeof(cTerminalInput));  // Convert String to Char Array
      char *cOption = strtok(cTerminalInput, ",");                         // Parsing the Option Flag
      int iCase = atoi(cOption);                                           // Converting Byte to Int for Case Loop


      // Cancel if Option input is invalid
      if (iCase < 1 || iCase > 9) {
        Serial.println("Invalid <Option>");
        Serial.println("Please consider the syntax: <option>,<value>");
        return;
      } else {
        // go forward if Option could successfull parsed
        if (cOption) {
          char *cValue = strtok(NULL, " ");
          byte byValue = byte(atoi(cValue));
          String sError;

          //Depending of the option the value input will checked
          do {
            if (cValue) {
              // Set year
              if (iCase == 1) {
                if (byValue < 0 || byValue > 99) {
                  iCase = 100;
                  sError = "Valid input range: 0 - 99";
                } else {
                  rtcDs3231.setYear(byValue);
                  iCase = 10;
                }
                // Set Month
              } else if (iCase == 2) {
                if (byValue < 1 || byValue > 12) {
                  iCase = 100;
                  sError = "Valid input range: 1 - 12";
                } else {
                  rtcDs3231.setMonth(byValue);
                  iCase = 10;
                }
                // Set Day of Month
              } else if (iCase == 3) {
                byte byMonth = rtcDs3231.getMonth(xCentury);
                if (byValue < 1 || byValue > 31) {
                  iCase = 100;
                  sError = "Valid input range: 1-31, depending on the months";
                } else {
                  //Leapcear calculation for february
                  bool xLeapYear = false;
                  byte byYear = rtcDs3231.getYear();
                  byte byLeapYearConditions[3];
                  byLeapYearConditions[0] = byYear % 4;
                  byLeapYearConditions[1] = byYear % 100;
                  byLeapYearConditions[2] = byYear % 400;
                  Serial.println(byLeapYearConditions[0]);
                  Serial.println(byLeapYearConditions[1]);
                  Serial.println(byLeapYearConditions[2]);

                  //Basically all Year which divisible with 4 are leapyears
                  if (byLeapYearConditions[0] == 0) {
                    xLeapYear = true;
                    //Years which divisible with 4 and 100 are no leapyears
                    if (byLeapYearConditions[1] == 0) {
                      xLeapYear = false;
                      // But if the year divisible with 100 && 400 it's a leapyear yet
                      if (byLeapYearConditions[2] == 0) {
                        xLeapYear = true;
                      }
                    }
                  }
                  Serial.println(xLeapYear);

                  if ((byMonth == 1 || byMonth == 5 || byMonth == 7 || byMonth == 8 || byMonth == 10 || byMonth == 12) && byValue <= 31 ) {
                    rtcDs3231.setDate(byValue);
                    iCase = 10;
                  } else if (byMonth == 2 && xLeapYear == true) {
                    if (byValue > 29 )
                      iCase = 100;
                    else {
                      rtcDs3231.setDate(byValue);
                      iCase = 10;
                    }
                  } else if (byMonth == 2 && xLeapYear == false) {
                    if (byValue > 28 )
                      iCase = 100;
                    else {
                      rtcDs3231.setDate(byValue);
                      iCase = 10;
                    }
                  } else if ((byMonth == 4 || byMonth == 6 || byMonth == 9 || byMonth == 11) && byValue <= 30) {
                    if (byValue > 30 )
                      iCase = 100;
                    else {
                      rtcDs3231.setDate(byValue);
                      iCase = 10;
                    }
                  }

                }
              } // Set Day of Week
              else if (iCase == 4) {
                if (byValue < 1 || byValue > 7) {
                  iCase = 100;
                  sError = "Valid input range: 1 - 7";
                } else {
                  rtcDs3231.setDoW(byValue);
                  iCase = 10;
                }
                // Set Time: Hour
              } else if (iCase == 5) {
                if (byValue < 0 || byValue > 23) {
                  iCase = 100;
                  sError = "Valid input range: 0 - 23";
                } else {
                  rtcDs3231.setHour(byValue);
                  iCase = 10;
                }
                // Set Time: Minutes
              } else if (iCase == 6) {
                if (byValue < 0 || byValue > 59) {
                  iCase = 100;
                  sError = "Valid input range: 0 - 59";
                } else {
                  rtcDs3231.setMinute(byValue);
                  iCase = 10;
                }
                // Set Time: Seconds
              } else if (iCase == 7) {
                if (byValue < 0 || byValue > 59) {
                  iCase = 100;
                  sError = "Valid input range: 0 - 59";
                } else {
                  rtcDs3231.setSecond(byValue);
                  iCase = 10;
                }
                // Set Clockmode: AM/PM or 24h mode
              } else if (iCase == 8) {
                if (byValue < 0 || byValue > 1) {
                  iCase = 100;
                  sError = "Valid input range: 0 = 12h / 1 = 24h";
                } else {
                  rtcDs3231.setClockMode(byValue);
                  iCase = 10;
                }
                // Cancel the setup routine
              } else if (iCase == 9) {
                Serial.println("Setup canceled...");
                return;
                // Handling correct insertion and asking for furthermore changes
              } else if (iCase == 10) {
                rtcLib.now();
                Serial.println("The RTC was set on: ");
                Serial.print(rtcDs3231.getYear());
                Serial.print("/");
                Serial.print(rtcDs3231.getMonth(xCentury));
                Serial.print("/");
                Serial.println(rtcDs3231.getDate());
                Serial.print("Current Day of Week: ");
                Serial.println(rtcDs3231.getDoW());
                Serial.print("Current Time: ");
                Serial.print(rtcDs3231.getHour(xH12, xhPM));
                Serial.print(" : ");
                Serial.print(rtcDs3231.getMinute());
                Serial.print(" : ");
                Serial.println(rtcDs3231.getSecond());

                Serial.print("Do you want change more option?");
                Serial.println("(Y/N)");

                while (Serial.available() <= 0)
                  ;

                if (Serial.available() > 0) {
                  char cYesNo = Serial.read();
                  if (cYesNo == 'N' || cYesNo == 'n') {
                    return;
                  } else if (cYesNo == 'Y' || cYesNo == 'y') {
                    xSetupActive = true;
                    iCase = 0;
                    Serial.end();
                  }
                }
              } else if (iCase == 100) {
                Serial.println("Invalid <value>");
                Serial.println("Please consider the syntax: <option>,<value>");
                Serial.end();
                xSetupActive = true;
                iCase = 0;
              }
            }
          } while (iCase == 10 || iCase == 100);
        }
      }
    }
  } while (xSetupActive == true);
}


// Check whether day has changed;
// -> if yes, return true
bool rtcCheckDayChange() {
  byDateActual = rtcDs3231.getDate();

  if (byDateActual != byDateOld) {
    byDateOld = byDateActual;
    return true;
  } else
    return false;
}