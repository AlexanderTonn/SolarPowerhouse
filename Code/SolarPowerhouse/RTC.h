#ifndef RTC_H
#define RTC_H

#include <DS3231.h>
#include <Wire.h>

// DS3231 Variables
DS3231 rtcDs3231;
RTClib rtcLib;
bool xCentury;
bool xH12;  // 12/24h Flag
bool xhPM;  // AM/PM Flag
byte byDateOld = 0;         // needed for detecting whether day has changed
bool xDateChanged = false;  // true, if Date has changed

void rtcInit();
auto rtcCheckDayChange(bool xSettDayChangeActive) -> bool;

#endif