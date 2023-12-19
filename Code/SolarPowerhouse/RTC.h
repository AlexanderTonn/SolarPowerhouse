#ifndef RTC_H
#define RTC_H

#include <NTPClient.h>
#include <mbed_mktime.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "opta_info.h"

// https://www.tutorialspoint.com/c_standard_library/time_h.htm

#if __has_include("opta_info.h")
        
        #define GET_OPTA_OTP_BOARD_INFO
        OptaBoardInfo* info;
        OptaBoardInfo* boardInfo();
 #endif

class rtc
{
    private:
    EthernetUDP ntpUdp;

    byte byMacAddress[6] = {0x00 , 0x00, 0x00, 0x00, 0x00, 0x00}; // MAC Address for Ethernet, only bad C-syntax possible
    constexpr static uint16_t PORT = 8888;

    public:
    NTPClient timeClient;
    rtc() : timeClient(ntpUdp, "pool.ntp.org", 3600, 0) {}

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

    auto getTimeData(timeOption to, debugInformations debug = debugInformations::NONE) -> uint16_t;
};

#endif