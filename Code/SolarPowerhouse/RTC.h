#ifndef RTC_H
#define RTC_H

#include <NTPClient.h>
#include <mbed_mktime.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// https://www.tutorialspoint.com/c_standard_library/time_h.htm

class rtc
{
    EthernetUDP ntpUdp;
    NTPClient timeClient;

public:
    rtc() : timeClient(ntpUdp, "pool.ntp.org", -6 * 3600, 0) {}

private:
public:
    enum class debugInformations
    {
        NONE,
        ACTIVE
    };
    enum class timeOption
    {
        SECONDS,
        MINUTES,
        HOURS,
        DAY_OF_MONTH,
        MONTH,
        YEAR,
        DAY_OF_WEEK,
        DAY_OF_YEAR,
        DAYLIGHT_SAVING_TIME
    };

    bool xDateChanged = false; // true, if Date has changed

    auto init() -> void;
    auto setNtpTime() -> void;
    auto rtcCheckDayChange(bool xSettDayChangeActive) -> bool;
    auto updateTime(debugInformations info = debugInformations::NONE) -> void;
    auto getLocalTime() -> String;
    auto getLocalHour() -> uint8_t;

    auto getTimeData(timeOption to) -> uint16_t;
};

#endif