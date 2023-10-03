#ifndef CONTROL_LOGIC_HPP
#define CONTROL_LOGIC_HPP

#include "RTC.h"
#include "pinDefinitions.h"
#include "HMI.h"

uint32_t uiBaudrate = 115200;

// Control Logic
bool xFirstCycle = true;                // for all function which should only execute once
float fBatteryVoltage = 0.0;            // measured voltage of battery
float fTrigBatteryFullPrevious = 0.0;   //used for edge detection
float fTrigBatteryEmptyPrevious = 0.0;  //used for edge detection
bool xSwitchLaterOnMppt = false;    //Switch later if current certainly to high
bool xSwitchLaterOnInverter = false;    //Switch later if current certainly to high

bool xSett_PVonInverter = false;     // Settings option for permanently connecting PV to Inverter
bool xSett_PVonMppt = false;    // Settings option for permanently connecting PV to MPPT
bool xSett_ResetDay = true;  // Settings option for switch back PV1 onto battery after day change
uint8_t uiSett_PVswitchingDelay = 2;  // Settings option for delay between switching PV1 from battery to inverter
bool xSett_considerCurrent = true; // Settings option for considering current for switching PV1

uint32_t uiMillisCheckDayChange = 0 ;           // 1 hour 

enum operationMode{
    NORMAL,
    BATTERY_MODE,
    INVERTER_MODE,
    NO_DAY_CHANGE
};

float fSolarVoltage = 0.0; 
float fSett_TargetVoltageInverter = 27.00;  // Target Voltage for switching to Inverter
float fSett_TargetVoltageMppt = 26.00;      // Target Voltage for switching back to Mppt
float fSett_SwitchCurrentTarget = 1.00;       // Threshold Current when is switching allowed
uint64_t uiMyMillis = 0;

auto pv1OnMppt() -> void;           // Setting relay outputs, have to be run in void loop()
auto pv1OnInverter() -> void;       // Setting relay outputs, have to be run in void loop()
bool xPv1OnMppt = false;            // marker bit for switching PV1 to MPPT
bool xPv1OnInverter = false;        // marker bit for switching PV1 to Inverter

auto SettingsPvSwitching(bool xOnInverter, bool xOnMppt) -> operationMode;     // checking available options for switching PV1
auto controlLogic() -> void;                                                  // main function for control logic

#endif
