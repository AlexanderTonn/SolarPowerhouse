#ifndef MODBUSRTU_TOYO_H
#define MODBUSRTU_TOYO_H

#include <ArduinoRS485.h> // https://www.arduino.cc/reference/en/libraries/arduinors485/
#include <ArduinoModbus.h> // https://www.arduino.cc/reference/en/libraries/arduinomodbus/

class ModbusRTU_Toyo
{
    private:

     auto convertToASCII(String s) -> String;
     auto hexToDec(uint16_t uiHex) -> uint16_t;
     auto hexToFlo(uint16_t uiHex) -> float;

     enum class debugSettings
     {
         NONE,
         PRINT_RAW,
         PRINT_POLL_ERROR,
         PRINT_WRITE_ERROR
     };
     enum class bytePos
     {
        LOW_BYTE,
        HIGH_BYTE
     };
     template <typename T>
     auto printDebug(debugSettings dSetting, T debugVariable, String sFunction) -> void;

     auto selectByte(bytePos pos, uint16_t uiInput) -> uint8_t;

    public:

    enum class debugLevel
    {
        NONE,
        ACTIVE
    };
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
    enum class chargeAndLoadStates
    {
        LOAD_OUTPUT_ACTIVE,
        IDLE,
        BOOST,
        EQUALIZE,
        FLOAT,
        OTHER
    };

    auto init(uint16_t baudrate) -> bool; // init ModbusRTU 
    auto getSysVoltAndCurrent(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> String; 
    auto getProductModel(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> String; 
    auto getSwVersion(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> String; 
    auto getHwVersion(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> String; 
    auto getSn(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE, edited edit = edited::RAW) -> String;  
    
    auto getBatSoC(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint8_t; 
    auto getBatVolt (uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> float;
    auto getBatCurr (uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> float;
    auto getBatSurTemp(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint8_t;
    auto getDevTemp(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint8_t;
    auto getLoadVolt(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> float;
    auto getLoadCurr(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> float;
    auto getLoadPwr(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint16_t;
    auto getSolarVolt(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> float;
    auto getSolarCurr(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> float;
    auto getSolarPwr(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint16_t;
    auto setLoadState(uint8_t uiDevice, status set, debugLevel dLevel = debugLevel::NONE) -> void;
    auto getBatMinVoltToday(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> float;
    auto getBatMaxVoltToday(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> float;
    auto getMaxChargCurrToday(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> float;
    auto getMaxLoadCurrToday(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> float;
    auto getMaxChargPwrToday(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint16_t;
    auto getMaxLoadPwrToday(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint16_t;
    auto getChargAhToday(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint16_t;
    auto getLoadAhToday(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint16_t;
    auto getPwrGenerationToday(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint16_t;
    auto getPwrConsumptionToday(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint16_t;
    auto getTotalDaysOfOperation(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint16_t;
    auto getTotalNoOverDischarges(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint16_t;
    auto getTotalNoFullCharges(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint16_t;
    auto getTotalAmpHoursCharged(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint32_t;
    auto getTotalAmpHoursDrawn(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint32_t;
    auto getCumulativeKwhGeneration(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint32_t;
    auto getCumulativeKwhDrawn(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> uint32_t;
    auto getLoadChargeStateBits(uint8_t uiDevice, debugLevel dLevel = debugLevel::NONE) -> String;

    //TODO!



};


#endif // RS485_HANDLER_H