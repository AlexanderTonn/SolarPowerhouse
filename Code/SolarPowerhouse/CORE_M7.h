#ifndef CORE_M7_HPP
#define CORE_M7_HPP

#include "pinDefinitions.h"
#include "modbusRTU_toyo.h"
#include "atUtility.h"
#include "rtc.h"


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

    constexpr static  byte BAT_SOC_ARRAY_SIZE = 15;
    Array <float, BAT_SOC_ARRAY_SIZE> aSocVoltage24 = {{20.0, 22.4, 24.0, 25.2, 25.6, 25.8, 26.0, 26.1, 26.2, 26.4, 26.6, 26.8, 27.0, 27.6, 29.2}};
    Array <float, BAT_SOC_ARRAY_SIZE> aSoCPercentage = {{0.0, 5.0, 9.5, 14.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 99.0, 99.5, 100.0}};
    Array <bool, 4> aRelayStates = {false}; // 0 = K1, 1 = K2, 2 = K3, 3 = K4

    auto SettingsPvSwitching(bool xOnInverter, bool xOnMppt) -> operationMode;     // checking available options for switching PV1
    auto calculateSoC() -> float;    // calculating SoC
    auto warnLowBatVoltage() -> bool;   // warning if battery voltage is low
    

    public:
    ModbusRTU_Toyo mpptCharger;  
    rtc ntp;
    atUtilities util;

    byte byMacAddress[6] = {0x00 , 0x00, 0x00, 0x00, 0x00, 0x00}; // MAC Address for Ethernet, only bad C-syntax possible
    Array <uint32_t,3> aMillisOld; // Saving the old Millis values for functionTrigger
    
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
        float fTargetWarningVoltage = 25.80;    // Target Voltage for warning if battery voltage is low
        float fTariff = 0.38; // Energy price tariff 
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
    auto calculateEnergySavings(float fTariff, uint32_t uiWh) -> float;

    auto debugMessenger(String sMessage) -> String;    // debug messenger for cloud variables

};

CORE_M7 mainCore; // create object of control class

#endif
