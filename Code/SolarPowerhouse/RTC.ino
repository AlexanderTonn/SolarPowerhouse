#include "rtc.h"

/**
 * @brief
 * @param none
 * @return none
 */
auto rtc::init() -> void
{
#if defined GET_OPTA_OTP_BOARD_INFO
  info = boardInfo();
  if (info->magic == 0xB5)
  {
    for (auto i = 0; i < 6; i++)
    {
      byMacAddress[i] = info->mac_address[i];
    }
  }

  if (Ethernet.begin(byMacAddress) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    else if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Ethernet cable is not connected.");
    }
  }

  ntpUdp.begin(PORT);

  delay(1000);

  timeClient.begin();
  Serial.println("Time Client started");
  updateTime(debugInformations::ACTIVE);
#elif
  Serial.println("No Opta Board Info, could not Init RTC");
#endif
}

/**
 * @brief
 * Checks whether day has changed
 * -> if yes, return true
 * @param xSettDayChangeActive
 * @return bool
 */
auto rtc::rtcCheckDayChange(bool xSettDayChangeActive) -> bool
{
  static uint16_t uiDateOld = 0; // needed for detecting whether day has changed

  if (!xSettDayChangeActive)
  {
    return false;
  }
  // if xSettDayChangeActive is true, check whether day has changed
  else
  {
    uint16_t uiDayActual = getTimeData(timeOption::DAY_OF_YEAR);
    if (uiDayActual != uiDateOld)
    {
      uiDateOld = uiDayActual;
      return true;
    }
    else
      return false;
  }
}
/**
 * @brief Updates internal RTC of Arduino Opta by using NTPClient
 *
 * @param info
 */
auto rtc::updateTime(debugInformations info) -> void
{
  timeClient.update();
  const uint32_t epoch = timeClient.getEpochTime();
  set_time(epoch);

  if (info == debugInformations::ACTIVE)
  {
    Serial.println("UTC Time: " + String(getLocalTime()));
    Serial.println("Unix Time: " + String(epoch));
  }
}

/**
 * @brief Time as String in format "hh:mm:ss"
 *
 * @param none
 * @return Opta Time as String in format "hh:mm:ss"
 */
auto rtc::getLocalTime() -> String
{
  char buffer[32];
  tm t; // tm is a structure defined in time.h
  _rtc_localtime(time(NULL), &t, RTC_FULL_LEAP_YEAR_SUPPORT);
  strftime(buffer, sizeof(buffer), "%k:%M:%S", &t);
  return String(buffer);
}
/**
 * @brief
 *
 * @param timeOption enum class of rtc class
 * @param debugInformations enum class of rtc class
 * @return uinz16_t
 * /
 **/
auto rtc::getTimeData(timeOption to, debugInformations debug) -> uint16_t
{
  uint16_t uiValue = 0;

  tm t;
  _rtc_localtime(time(NULL), &t, RTC_FULL_LEAP_YEAR_SUPPORT);

  switch (to)
  {
  case timeOption::SECONDS:
    uiValue = static_cast<uint16_t>(t.tm_sec);
    break;

  case timeOption::MINUTES:
    uiValue = static_cast<uint16_t>(t.tm_min);
    break;

  case timeOption::HOURS:
    uiValue = static_cast<uint16_t>(t.tm_hour);
    break;

  case timeOption::DAY_OF_MONTH:
    uiValue = static_cast<uint16_t>(t.tm_mday);
    break;

  case timeOption::MONTH:
    uiValue = static_cast<uint16_t>(t.tm_mon);
    break;

  case timeOption::YEAR:
    uiValue = static_cast<uint16_t>(t.tm_year);
    break;

  case timeOption::DAY_OF_WEEK:
    uiValue = static_cast<uint16_t>(t.tm_wday);
    break;

  case timeOption::DAY_OF_YEAR:
    uiValue = static_cast<uint16_t>(t.tm_yday);
    break;

  case timeOption::DAYLIGHT_SAVING_TIME:
    uiValue = static_cast<uint16_t>(t.tm_isdst);
    break;

  default:
    Serial.println("No valid option passed");
    break;
  }

  if (debug == debugInformations::ACTIVE)
  {
    Serial.println("Time: " + String(uiValue));
  }
}
