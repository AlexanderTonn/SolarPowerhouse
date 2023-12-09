
#include "modbusRTU_toyo.h"

auto ModbusRTU_Toyo::init(uint16_t baud) -> bool
{
    if (!Serial)
    {
        Serial.begin(baud);
    }

    if (!ModbusRTUClient.begin(baud))
    {
        Serial.println("Failed to start Modbus RTU Client!");
        return false;
    }
    else
        return true;
}

// Get the selected System Voltage and Current
auto ModbusRTU_Toyo::getSysVoltAndCurrent(uint8_t uiDevice, debugLevel dLevel) -> String
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x0A))
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
        else
        {
            if (ModbusRTUClient.available())
            {
                uint16_t uiBuffer;
                while (ModbusRTUClient.available())
                    uiBuffer = ModbusRTUClient.read();

                uint8_t uiLowByte = (uiBuffer >> 8) & 0xFF;
                uint8_t uiHighByte = uiBuffer & 0xFF;

                String sResult = "System voltage: " + String(uiHighByte) + " V / System Current: " + String(uiLowByte) + " A";

                if (dLevel == debugLevel::ACTIVE)
                {
                    printDebug(debugSettings::PRINT_RAW, uiLowByte, __func__);
                    printDebug(debugSettings::PRINT_RAW, uiHighByte, "");
                }
                return sResult;
            }
        }
}
// Get the name of the used mppt charger
// 8 Registers to read (16 bytes)
// the values are stored in following ASCII format (example):
// ’’, ' ', ' ', ' ', 'M', 'T', '4', '8', '3' , '0' , ' ', ' ', ' ', ' ', ' ', ' '
auto ModbusRTU_Toyo::getProductModel(uint8_t uiDevice, debugLevel dLevel) -> String
{
    if (!ModbusRTUClient.requestFrom(uiDevice, 0x0C, 8))
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
        else
        {
            String sRead;
            while (ModbusRTUClient.available())
            {
                sRead += ModbusRTUClient.read();
            }
            String sResult = convertToASCII(sRead);

            return sResult;
        }
}
// Reading software version
// highest byte is not used
auto ModbusRTU_Toyo::getSwVersion(uint8_t uiDevice, debugLevel dLevel) -> String
{
    if (!ModbusRTUClient.requestFrom(uiDevice, 0x014, 2))
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
        else
        {
            Array<uint16_t, 2> uiBuffer;
            byte byCounter = 0;
            while (ModbusRTUClient.available())
            {
                uiBuffer.at(byCounter) = ModbusRTUClient.read();
                byCounter++;
            }

            // Parsing
            // High Byte 0x00 of first register is not used
            uint8_t uiFirstVersionNo = (uiBuffer.at(0) >> 8) & 0xFF;
            uint8_t uiThirdVersionNo = (uiBuffer.at(1) >> 8) & 0xFF;
            uint8_t uiSecondVersionNo = uiBuffer.at(1) & 0xFF;

            if (dLevel == debugLevel::ACTIVE)
            {
                printDebug(debugSettings::PRINT_RAW, uiFirstVersionNo, __func__);
                printDebug(debugSettings::PRINT_RAW, uiSecondVersionNo, "");
                printDebug(debugSettings::PRINT_RAW, uiThirdVersionNo, "");
            }

            return "V" + String(uiFirstVersionNo) + "." + String(uiSecondVersionNo) + "." + String(uiThirdVersionNo);
        }
}
auto ModbusRTU_Toyo::getHwVersion(uint8_t uiDevice, debugLevel dLevel) -> String
{
    if (!ModbusRTUClient.requestFrom(uiDevice, 0x016, 2))
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
        else
        {
            Array<uint16_t, 2> uiBuffer;
            byte byCounter = 0;
            while (ModbusRTUClient.available())
            {
                uiBuffer.at(byCounter) = ModbusRTUClient.read();
                byCounter++;
            }

            // Parsing
            // High Byte 0x00 of first register is not used
            uint8_t uiFirstVersionNo = (uiBuffer.at(0) >> 8) & 0xFF;
            uint8_t uiThirdVersionNo = (uiBuffer.at(1) >> 8) & 0xFF;
            uint8_t uiSecondVersionNo = uiBuffer.at(1) & 0xFF;

            if (dLevel == debugLevel::ACTIVE)
            {
                printDebug(debugSettings::PRINT_RAW, uiFirstVersionNo, __func__);
                printDebug(debugSettings::PRINT_RAW, uiSecondVersionNo, "");
                printDebug(debugSettings::PRINT_RAW, uiThirdVersionNo, "");
            }

            return "V" + String(uiFirstVersionNo) + "." + String(uiSecondVersionNo) + "." + String(uiThirdVersionNo);
        }
}
// Get S/N
// Format example: 1501FFFFH is the product serial number, indicating it's the 65535th (hexadecimal FFFFH) unit produced in Jan. of 2015
// Decide with enum class whether you want to get the raw String or the parsed String
auto ModbusRTU_Toyo::getSn(uint8_t uiDevice, debugLevel dLevel, edited edit) -> String
{
    if (!ModbusRTUClient.requestFrom(uiDevice, 0x018, 2))
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
        else
        {
            Array<uint16_t, 2> uiBuffer;
            byte byCounter = 0;
            while (ModbusRTUClient.available())
            {
                uiBuffer.at(byCounter) = ModbusRTUClient.read();
                byCounter++;
            }

            // Parsing
            // uiBuffer.at(0) == Device no of the year in which the device was produced
            uint8_t uiYear = (uiBuffer.at(0) >> 8) & 0xFF;
            uint8_t uiMonth = uiBuffer.at(0) & 0xFF;
            uint16_t uiDeviceNo = uiBuffer.at(1);

            if (dLevel == debugLevel::ACTIVE)
            {
                printDebug(debugSettings::PRINT_RAW, uiYear, __func__);
                printDebug(debugSettings::PRINT_RAW, uiMonth, "");
                printDebug(debugSettings::PRINT_RAW, uiDeviceNo, "");
            }

            if (edit == edited::RAW)
            {
                return String(uiYear) + String(uiMonth) + String(uiDeviceNo);
            }
            else
            {
                return "Manufacturing year: 20" + String(uiYear) + " Month: " + String(uiMonth) + " Device No: " + String(uiDeviceNo);
            }
        }
}

// Battery SoC in decimal percent (0-100)
// unit: %
// Register 0x100
auto ModbusRTU_Toyo::getBatSoC(uint8_t uiDevice, debugLevel dLevel) -> uint8_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x100))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

        return selectByte(bytePos::LOW_BYTE, uiBuffer);
    }
}

// battery voltage
// unit: V
// Register 0x101
auto ModbusRTU_Toyo::getBatVolt(uint8_t uiDevice, debugLevel dLevel) -> float
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x101))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        auto buffer = 0.0f;
        while (ModbusRTUClient.available())
            buffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, buffer, __func__);

        return buffer * 0.1;
    }
}

// Battery current
// unit: A
// Register: 0x102
auto ModbusRTU_Toyo::getBatCurr(uint8_t uiDevice, debugLevel dLevel) -> float
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x102))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        auto buffer = 0.0f;
        while (ModbusRTUClient.available())
            buffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, buffer, __func__);

        return buffer * 0.01;
    }
}
// Battery surface temperature
// if the figure turns out to be 800AH, then it indicates the battery's surface temperature is -10 °C
// unit: °C
// Register: 0x103, highbyte
auto ModbusRTU_Toyo::getBatSurTemp(uint8_t uiDevice, debugLevel dLevel) -> uint8_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x103))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {

        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

        return selectByte(bytePos::LOW_BYTE, uiBuffer);
    }
}

// Battery surface temperature
// if the figure turns out to be 800AH, then it indicates the battery's surface temperature is -10 °C
// unit: °C
// Register: 0x103, Lowbyte
auto ModbusRTU_Toyo::getDevTemp(uint8_t uiDevice, debugLevel dLevel) -> uint8_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x103))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

        return selectByte(bytePos::HIGH_BYTE, uiBuffer);
    }
}

// Load Volt
// unit: V
// Register: 0x104
auto ModbusRTU_Toyo::getLoadVolt(uint8_t uiDevice, debugLevel dLevel) -> float
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x104))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        auto buffer = 0.0f;
        while (ModbusRTUClient.available())
            buffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, buffer, __func__);

        return buffer * 0.1;
    }
}

// Load current
// unit: A
// Register: 0x105
auto ModbusRTU_Toyo::getLoadCurr(uint8_t uiDevice, debugLevel dLevel) -> float
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x105))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        auto buffer = 0.0f;
        while (ModbusRTUClient.available())
            buffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, buffer, __func__);

        return buffer * 0.01;
    }
}
// Load power
// unit: W
// Register: 0x106
auto ModbusRTU_Toyo::getLoadPwr(uint8_t uiDevice, debugLevel dLevel) -> uint16_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x106))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

        return uiBuffer;
    }
}
// Solar Voltage
// unit: V
// Register: 0x107
auto ModbusRTU_Toyo::getSolarVolt(uint8_t uiDevice, debugLevel dLevel) -> float
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x107))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }

    else
    {
        auto buffer = 0.0f;
        while (ModbusRTUClient.available())
            buffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, buffer, __func__);

        return buffer * 0.1;
    }
}
// Solar current
// unit: A
// Register: 0x108
auto ModbusRTU_Toyo::getSolarCurr(uint8_t uiDevice, debugLevel dLevel) -> float
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x108))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }

    else
    {
        auto buffer = 0.0f;
        while (ModbusRTUClient.available())
            buffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, buffer, __func__);

        return buffer * 0.01;
    }
}
// Solar power
// unit: W
// Register: 0x109
auto ModbusRTU_Toyo::getSolarPwr(uint8_t uiDevice, debugLevel dLevel) -> uint16_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x109))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }

    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

        return uiBuffer;
    }
}
// Set load state
// 0x00: Turn off load
// 0x01: Turn on load
// Register: 0x10A
auto ModbusRTU_Toyo::setLoadState(uint8_t uiDevice, status set, debugLevel dLevel) -> void
{
    switch (set)
    {
    case status::OFF:
        if (!ModbusRTUClient.holdingRegisterWrite(uiDevice, 0x10A, 0x00))
        {
            if (dLevel == debugLevel::ACTIVE)
                printDebug(debugSettings::PRINT_WRITE_ERROR, 0, __func__);
        }
        else
        {
            if (dLevel == debugLevel::ACTIVE)
                printDebug(debugSettings::PRINT_RAW, 0x00, __func__);
        }
        break;
    case status::ON:
        if (!ModbusRTUClient.holdingRegisterWrite(uiDevice, 0x10A, 0x01))
        {
            if (dLevel == debugLevel::ACTIVE)
                printDebug(debugSettings::PRINT_WRITE_ERROR, 0, __func__);
        }
        else
        {
            if (dLevel == debugLevel::ACTIVE)
                printDebug(debugSettings::PRINT_RAW, 0x01, __func__);
        }
        break;
    }
}
// Get lowest battery voltage of today
// unit: V
// Register: 0x10B
auto ModbusRTU_Toyo::getBatMinVoltToday(uint8_t uiDevice, debugLevel dLevel) -> float
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x10B))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }

    else
    {
        auto buffer = 0.0f;
        while (ModbusRTUClient.available())
            buffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, buffer, __func__);

        return buffer * 0.1;
    }
}
// Get highest battery voltage of today
// unit: V
// Register: 0x10C
auto ModbusRTU_Toyo::getBatMaxVoltToday(uint8_t uiDevice, debugLevel dLevel) -> float
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x10C))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }

    else
    {
        auto buffer = 0.0f;
        while (ModbusRTUClient.available())
            buffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, buffer, __func__);

        return buffer * 0.1;
    }
}
// Get highest charging current of today
// unit: A
// Register: 0x10D
auto ModbusRTU_Toyo::getMaxChargCurrToday(uint8_t uiDevice, debugLevel dLevel) -> float
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x10D))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }

    else
    {
        auto buffer = 0.0f;
        while (ModbusRTUClient.available())
            buffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, buffer, __func__);

        return buffer * 0.01;
    }
}
// Get highest load (discharge) current of today
// unit: A
// Register: 0x10E
auto ModbusRTU_Toyo::getMaxLoadCurrToday(uint8_t uiDevice, debugLevel dLevel) -> float
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x10E))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }

    else
    {
        auto buffer = 0.0f;
        while (ModbusRTUClient.available())
            buffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, buffer, __func__);

        return buffer * 0.01;
    }
}
// Get highest charing power of today
// unit: W
// Register: 0x10F
auto ModbusRTU_Toyo::getMaxChargPwrToday(uint8_t uiDevice, debugLevel dLevel) -> uint16_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x10F))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }

    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

        return uiBuffer;
    }
}
// Get highest load (discharge) power of today
// unit: W
// Register: 0x110
auto ModbusRTU_Toyo::getMaxLoadPwrToday(uint8_t uiDevice, debugLevel dLevel) -> uint16_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x110))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }

    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

        return uiBuffer;
    }
}
// get charging ampere hours of today
// unit: Ah
// Register: 0x111
auto ModbusRTU_Toyo::getChargAhToday(uint8_t uiDevice, debugLevel dLevel) -> uint16_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x111))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }

    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

        return uiBuffer;
    }
}
// get load (discharge) ampere hours of today
// unit: Ah
// Register: 0x112
auto ModbusRTU_Toyo::getLoadAhToday(uint8_t uiDevice, debugLevel dLevel) -> uint16_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x112))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }

    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

        return uiBuffer;
    }
}
// get power generation of today
// unit: kWh
// Register: 0x113
auto ModbusRTU_Toyo::getPwrGenerationToday(uint8_t uiDevice, debugLevel dLevel) -> uint16_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x113))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

        return uiBuffer;
    }
}
// get power consumption of today
// unit: kWh
// Register: 0x114
auto ModbusRTU_Toyo::getPwrConsumptionToday(uint8_t uiDevice, debugLevel dLevel) -> uint16_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x114))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

        return uiBuffer;
    }
}
// total Number of operating days
// unit: days
// Register: 0x115
auto ModbusRTU_Toyo::getTotalDaysOfOperation(uint8_t uiDevice, debugLevel dLevel) -> uint16_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x115))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        return uiBuffer;
    }
}
// get total numbers of over discharges
// unit: times
// Register: 0x116
auto ModbusRTU_Toyo::getTotalNoOverDischarges(uint8_t uiDevice, debugLevel dLevel) -> uint16_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x116))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        return uiBuffer;
    }
}
// get total numbers of full charges
// unit: times
// Register: 0x117
auto ModbusRTU_Toyo::getTotalNoFullCharges(uint8_t uiDevice, debugLevel dLevel ) -> uint16_t
{
    if (!ModbusRTUClient.holdingRegisterRead(uiDevice, 0x117))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {
        uint16_t uiBuffer = 0;
        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        return uiBuffer;
    }
}
// get Total charging amp hours
// unit: Ah
// Register: 0x118
auto ModbusRTU_Toyo::getTotalAmpHoursCharged(uint8_t uiDevice, debugLevel dLevel) -> uint32_t
{
    if (!ModbusRTUClient.requestFrom(uiDevice, 0x118, 2))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {

        uint32_t uiBuffer = 0;

        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        return uiBuffer;
    }
}
// get Total discharge amp hours
// unit: Ah
// Register: 0x11A
auto ModbusRTU_Toyo::getTotalAmpHoursDrawn(uint8_t uiDevice, debugLevel dLevel) -> uint32_t
{
    if (!ModbusRTUClient.requestFrom(uiDevice, 0x11A, 2))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {

        uint32_t uiBuffer = 0;

        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        return uiBuffer;
    }
}
// get cumulative kWh generation
// unit: kWh
// Register: 0x11C
auto ModbusRTU_Toyo::getCumulativeKwhGeneration(uint8_t uiDevice, debugLevel dLevel ) -> uint32_t
{
    if (!ModbusRTUClient.requestFrom(uiDevice, 0x11C, 2))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {

        uint32_t uiBuffer = 0;

        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        return uiBuffer;
    }

}
// get cumulative kWh drawn
// unit: kWh
// Register: 0x11E
auto ModbusRTU_Toyo::getCumulativeKwhDrawn(uint8_t uiDevice, debugLevel dLevel ) -> uint32_t
{
    if (!ModbusRTUClient.requestFrom(uiDevice, 0x11E, 2))
    {
        if (dLevel == debugLevel::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
    }
    else
    {

        uint32_t uiBuffer = 0;

        while (ModbusRTUClient.available())
            uiBuffer = ModbusRTUClient.read();

        return uiBuffer;
    }

}
// get charge and load status
// unit: kWh
// Register: 0x11C


auto ModbusRTU_Toyo::convertToASCII(String s) -> String
{
    String sResult;
    for (byte i = 0; i < s.length(); i++)
    {
        uint16_t uiASCII = s.substring(i, i + 1).toInt();
        sResult += char(uiASCII);
    }
    return sResult;
}

// Hex to Decimal conversion
auto ModbusRTU_Toyo::hexToDec(uint16_t uiHex) -> uint16_t
{
    return strtoul(String(uiHex, HEX).c_str(), NULL, 16);
}
// hex to float conversion
auto ModbusRTU_Toyo::hexToFlo(uint16_t uiHex) -> float
{
    uint16_t uiConverted = strtoul(String(uiHex, HEX).c_str(), NULL, 16);
    return static_cast<float>(uiConverted);
}

// Print debug output on serial monitor depending of the passed settings
template <typename T>
auto ModbusRTU_Toyo::printDebug(debugSettings dSetting, T debugVariable, String sFunctionName) -> void
{
    switch (dSetting)
    {
    case debugSettings::PRINT_RAW:
        Serial.println("Raw Value of " + sFunctionName + " :" + String(debugVariable));
        break;
    case debugSettings::PRINT_POLL_ERROR:
        Serial.print("Failed to request while" + sFunctionName + " | Error Code : ");
        Serial.println(ModbusRTUClient.lastError());
        break;
    case debugSettings::PRINT_WRITE_ERROR:
        Serial.print("Failed to write while" + sFunctionName + " | Error Code : ");
        Serial.println(ModbusRTUClient.lastError());
        break;
    default:
        Serial.println("No debug setting selected!");
        break;
    }
}

// Select the high or low byte of a 16 bit value
// Low Byte: 0x00FF
// High Byte: 0xFF00
auto ModbusRTU_Toyo::selectByte(bytePos pos, uint16_t uiInput) -> uint8_t
{
    switch (pos)
    {
    case bytePos::LOW_BYTE:
        return uiInput & 0xFF;
        break;
    case bytePos::HIGH_BYTE:
        return (uiInput >> 8) & 0xFF;
        break;
    default:
        return 0xFF;
        break;
    }
}