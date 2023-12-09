#ifndef CORE_M7_HPP
#define CORE_M7_HPP

#include "pinDefinitions.h"
#include "modbusRTU_toyo.h"
#include "sharedMemory.h"
#include "Util.h"
#include "rtc.h"
#include <Array.h>  // https://github.com/janelia-arduino/Array


class CORE_M7 
{
    private:
    ModbusRTU_Toyo mpptCharger;                  // create object of modbusRTU_toyo class
    sharedMemory ipc;                       // create object of sharedMemory class
    rtc ntp;

    uint32_t uiMillisCheckDayChange = 0 ;           // 1 hour 
    bool xFirstCycle = true;                // for all function which should only execute once
   
    byte byModeActual = 0;                  // actual operation mode
    byte byModeOld = 0;                     // old operation mode

    bool xPv1OnMppt = false;            // marker bit for switching PV1 to MPPT
    bool xPv1OnInverter = false;        // marker bit for switching PV1 to Inverter

    uint64_t uiMyMillis = 0;

    constexpr static uint8_t uiToyoDeviceId = 1;   // Modbus Device ID of MPPT Charger

    enum operationMode{
    NORMAL,
    BATTERY_MODE,
    INVERTER_MODE,
    NO_DAY_CHANGE
    };

    auto SettingsPvSwitching(bool xOnInverter, bool xOnMppt) -> operationMode;     // checking available options for switching PV1

    public:
    

    // Variables which are relevant for control logic
    struct processVariables
    {
        float fBatteryVoltage = 0.0;            // measured voltage of battery
        float fTrigBatteryFullPrevious = 0.0;   //used for edge detection
        float fTrigBatteryEmptyPrevious = 0.0;  //used for edge detection
        bool xSwitchLaterOnMppt = false;    //Switch later if current certainly to high
        bool xSwitchLaterOnInverter = false;    //Switch later if current certainly to high
        float fSolarCurrent = 0.0; // Current from Solar panels to mppt charger
    };

    // structfor getting informative informations which are not necessary for control Logic
    struct informativeVariables
    {
        float fSolarVoltage = 0.0; // solar voltage
        float fBatteryCurrent = 0.0; // battery current
        float fBatTodayVoltMin = 0.0; // min voltage of the day
        float fBatTodayVoltMax = 0.0; // max voltage of the day
        float fBatTodayCurrMax = 0.0; // max current of the day
        uint16_t uiCurrentSolarPower = 0; // current solar power
        uint16_t uiBatTodayPwrMax = 0; // max charge power of the day
        uint16_t uiBatTodayPwrGeneration = 0; // max generation (kWh) power of the day

        uint8_t uiBatteryTemperature = 0; // battery temperature
        uint8_t uiMpptChargerTemperature = 0; // mppt charger temperature

        bool xMpptChargerLoadActive = false; // mppt charger load 0 = off, 1 = on
    };

    // struct for settings
    struct settings
    {
        bool xPVonInverter = false;     // Settings option for permanently connecting PV to Inverter
        bool xPVonMppt = false;    // Settings option for permanently connecting PV to MPPT
        bool xResetDay = true;  // Settings option for switch back PV1 onto battery after day change
        uint8_t uiPVswitchingDelay = 2;  // Settings option for delay between switching PV1 from battery to inverter
        bool xignoreCurrent = false; // Settings option for considering current for switching PV1
        float fTargetVoltageInverter = 27.00;  // Target Voltage for switching to Inverter
        float fTargetVoltageMppt = 26.00;      // Target Voltage for switching back to Mppt
        float fSwitchCurrentTarget = 1.00;       // Threshold Current when is switching allowed
    };
    processVariables process;
    settings settings;
    informativeVariables informative;

    enum class debugLevel
    {
        NONE,
        ACTIVE
    };


    auto pv1OnMppt(debugLevel dLevel = debugLevel::NONE) -> void;           // Setting relay outputs, have to be run in void loop()
    auto pv1OnInverter(debugLevel dLevel = debugLevel::NONE) -> void;       // Setting relay outputs, have to be run in void loop()

    auto controlLogic() -> void;    // main function for control logic
    auto init() -> void;
    auto readInputs() -> void;   // reading inputs from MPPT Charger     
    auto readModbusRtu() -> void;   // reading inputs from MPPT Charger    
    auto writeOutputs() -> void;    
    auto writeSharedMemory() -> void;   // writing variables to shared memory

};

#endif
