#ifndef MODBUSRTU_TOYO_H
#define MODBUSRTU_TOYO_H

#include <ArduinoRS485.h>  // https://www.arduino.cc/reference/en/libraries/arduinors485/
#include <ArduinoModbus.h> // https://www.arduino.cc/reference/en/libraries/arduinomodbus/
#include "CORE_M7.h" 
#include "Array.h"
class ModbusRTU_Toyo
{
private:
    auto convertToASCII(String s) -> String;

    enum class debugSettings
    {
        NONE,
        PRINT_RAW,
        PRINT_POLL_ERROR,
        PRINT_WRITE_ERROR,
        PRINT_WRONG_VALUE_INPUT,
    };
    enum class bytePos
    {
        LOW_BYTE,
        HIGH_BYTE
    };

    /**
     * @brief internal option for choosing values
     */
    enum class description
    {
        CONTROLLER_INFOS,
        SYS_INFO
    };

    template <typename T>
    auto printDebug(debugSettings dSetting, T debugVariable, String sFunction) -> void;

    

public:

    enum class status
    {
        OFF,
        ON
    };
    enum class edited
    {
        RAW,
        EDITED
    };
    enum class language
    {
        EN,
        DE
    };


private:
    static constexpr std::size_t sizeControllerInfos = 35;
    Array<uint16_t, sizeControllerInfos> uiReadBuffer = {0};

    auto writeToStruct(description desc) -> void;

    template <std::size_t size>
    auto parseControllerInfos(Array<uint16_t, size> uiBuf) -> void;

    auto translateChargingState(uint8_t uiChargingState, uint8_t uiLang) -> String;
    uint8_t uiLanguage = 0; // 0 = EN, 1 = DE

public:
    enum class debugMode
    {
        NONE,
        ACTIVE
    };
    struct toyoInfo
    {
        uint8_t uiBatSoC;
        float fBatVolt;
        float fBatCurr;
        uint8_t uiBatSurTemp;
        uint8_t uiDevTemp;
        float fLoadVolt;
        float fLoadCurr;
        uint16_t uiLoadPwr;
        float fSolarVolt;
        float fSolarCurr;
        uint16_t uiSolarPwr;
        float fBatMinVoltToday;
        float fBatMaxVoltToday;
        float fMaxChargCurrToday;
        float fMaxLoadCurrToday;
        uint16_t uiMaxChargPwrToday;
        uint16_t uiMaxLoadPwrToday;
        uint16_t uiChargAhToday;
        uint16_t uiLoadAhToday;
        uint16_t uiPwrGenerationToday;
        uint16_t uiPwrConsumptionToday;
        uint16_t uiTotalDaysOfOperation;
        uint16_t uiTotalNoOverDischarges;
        uint16_t uiTotalNoFullCharges;
        uint32_t uiTotalAmpHoursCharged;
        uint32_t uiTotalAmpHoursDrawn;
        uint32_t uiCumulativeKwhGeneration;
        uint32_t uiCumulativeKwhDrawn;
        bool xLoadOutputActive; 
        uint8_t uiLoadBrightness; // TODO 0-100% but what does this value mean?
        uint8_t uiChargingState; 
        String sChargingState; 
    };
    toyoInfo controllerInformation;

    // Enum class for choosing values to write
    enum class toyoWritableValues
    {
        LOAD_OUTPUT
    };

    auto init(const uint16_t baudrate, ModbusRTU_Toyo::language lang = ModbusRTU_Toyo::language::EN) -> bool; // init ModbusRTU
    auto update(uint16_t uiReadInterval) -> void;

    template <std::size_t size>
    auto readContrInfo(uint16_t uiDevice,
                       uint16_t uiStartAddress,
                       Array<uint16_t, size> &uiData,
                       debugMode dLevel = debugMode::NONE) -> bool;

    auto writeContrInfo(uint16_t uiDevice,
                        toyoWritableValues targetVariable,
                        uint16_t uiValue,
                        debugMode dLevel = debugMode::NONE) -> void;
};

#endif // RS485_HANDLER_H