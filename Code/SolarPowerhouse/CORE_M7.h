#ifndef CORE_M7_HPP
#define CORE_M7_HPP

#include "pinDefinitions.h"
#include "modbusRTU_toyo.h"
#include "Util.h"
#include "rtc.h"
#include <Array.h>  // https://github.com/janelia-arduino/Array
#include "thingProperties.h"


class CORE_M7 
{
    private:

    uint32_t uiMillisCheckDayChange = 0 ;           // 1 hour 
    bool xFirstCycle = true;                // for all function which should only execute once
   
    byte byModeActual = 0;                  // actual operation mode
    byte byModeOld = 0;                     // old operation mode

    bool xPv1OnMppt = false;            // marker bit for switching PV1 to MPPT
    bool xPv1OnInverter = false;        // marker bit for switching PV1 to Inverter

    uint64_t uiMyMillis = 0;

    float fTrigBatteryFullPrevious = 0.0;   //used for edge detection
    float fTrigBatteryEmptyPrevious = 0.0;  //used for edge detection
    bool xSwitchLaterOnMppt = false;    //Switch later if current certainly to high
    bool xSwitchLaterOnInverter = false;    //Switch later if current certainly to high

    constexpr static uint8_t uiToyoDeviceId = 1;   // Modbus Device ID of MPPT Charger

    enum operationMode{
    NORMAL,
    BATTERY_MODE,
    INVERTER_MODE,
    NO_DAY_CHANGE
    };

    auto SettingsPvSwitching(bool xOnInverter, bool xOnMppt) -> operationMode;     // checking available options for switching PV1

    public:
    ModbusRTU_Toyo mpptCharger;  
    rtc ntp;
    
    // struct for settings
    struct settings
    {
        bool xPVonInverter = false;     // Settings option for permanently connecting PV to Inverter
        bool xPVonMppt = false;    // Settings option for permanently connecting PV to MPPT
        bool xResetDay = true;  // Settings option for switch back PV1 onto battery after day change
        uint8_t uiPVswitchingDelay = 2;  // Settings option for delay between switching PV1 from battery to inverter
        bool xignoreCurrent = false; // Settings option for considering current for switching PV1
        float fTargetVoltageInverter = 28.20;  // Target Voltage for switching to Inverter
        float fTargetVoltageMppt = 26.00;      // Target Voltage for switching back to Mppt
        float fSwitchCurrentTarget = 1.00;       // Threshold Current when is switching allowed
    };
    settings settings;

    enum class debugMode
    {
        NONE,
        ACTIVE
    };


    auto pv1OnMppt(debugMode dLevel = debugMode::NONE) -> void;           // Setting relay outputs, have to be run in void loop()
    auto pv1OnInverter(debugMode dLevel = debugMode::NONE) -> void;       // Setting relay outputs, have to be run in void loop()

    auto controlLogic() -> void;    // main function for control logic
    auto init() -> void;
    auto writeOutputs() -> void;    
    auto writeCloudVariables() -> void;   // writing variables to cloud variables

};

#endif
