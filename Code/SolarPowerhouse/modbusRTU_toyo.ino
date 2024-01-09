/**
 * @file modbusRTU_toyo.ino
 * @author https://github.com/AlexanderTonn
 * @brief Handling the communication with the Toyo Solar MPPT Charger
 * @brief similiar Products labeled as: SR-MLXXXX (e.g. SR-ML4860)
 * @version 0.1
 * @date 2023-12-11
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "modbusRTU_toyo.h"
/**
 * @brief Modbus RTU Init
 * ! Calculate preDelay and postDelay in microseconds as per Modbus RTU Specification
 * MODBUS over serial line specification and implementation guide V1.02
 * Paragraph 2.5.1.1 MODBUS Message RTU Framing
 * https://modbus.org/docs/Modbus_over_serial_line_V1_02.pdf
 * @param baud
 * @return true
 * @return false
 */
auto ModbusRTU_Toyo::init(const uint16_t baud, ModbusRTU_Toyo::language lang) -> bool
{
    const auto bitduration{1.f / baud};
    const auto preDelayBR{bitduration * 10.0f * 3.5f * 1e6};
    const auto postDelayBR{bitduration * 10.0f * 3.5f * 1e6};
    if (!Serial)
    {
        Serial.begin(baud);
    }
    Serial.println("Starting Modbus RTU Client! ");
    RS485.setDelays(preDelayBR, postDelayBR);

    switch (lang)
    {
    case ModbusRTU_Toyo::language::EN:
        uiLanguage = static_cast<uint8_t>(language::EN);
        break;
    case ModbusRTU_Toyo::language::DE:
        uiLanguage = static_cast<uint8_t>(language::DE);
        break;
    default:
        Serial.println("Passed Language Index is not existing!");
        break;
    }

    // Starting client
    if (!ModbusRTUClient.begin(baud, SERIAL_8N2))
    {
        Serial.println("Failed to start Modbus RTU Client!");
        return false;
    }
    else
        return true;
}
/**
 * @brief
 * Reading the Modbus RTU registers which giving informative data about the device
 * @tparam size
 * @param uiDevice  Modbus Device ID / Slave ID
 * @param uiStartAddress  No. of the first register
 * @param uiData std::array for buffering data
 * @param dLevel Serial Print of raw data
 * @return true or false
 */
template <std::size_t size>
auto ModbusRTU_Toyo::readContrInfo(uint16_t uiDevice,
                                   uint16_t uiStartAddress,
                                   Array<uint16_t, size> &uiData,
                                   debugMode dLevel) -> bool
{
    if (!ModbusRTUClient.requestFrom(uiDevice, HOLDING_REGISTERS, uiStartAddress, sizeof(uiData) / 2))
    {
        if (dLevel == debugMode::ACTIVE)
            printDebug(debugSettings::PRINT_POLL_ERROR, 0, __func__);
        return false;
    }
    else
    {
        uint16_t uiBuffer = 0;
        byte byCounter = 0;
        while (ModbusRTUClient.available())
        {
            uiBuffer = ModbusRTUClient.read();
            uiData.at(byCounter) = uiBuffer;

            if (dLevel == debugMode::ACTIVE)
                printDebug(debugSettings::PRINT_RAW, uiBuffer, __func__);

            byCounter++;
        }
        return true;
    }
}
/**
 * @brief depending of selected Option, the data is written to the struct
 *
 * @param desc
 * @return none
 */
auto ModbusRTU_Toyo::writeToStruct(description desc) -> void
{
    switch (desc)
    {
    case description::CONTROLLER_INFOS:
        parseControllerInfos(uiReadBuffer);
        break;

        // TODO!: ADD OTHER OPTIONS

    default:
        Serial.println("No valid Option for writeToStruct");
        break;
    }
}
/**
 * @brief Parsing Informative Controller data
 * ! data of modbusregister from 0x100 to 0x11F
 *
 * @param uiBuf
 * @return none
 */
template <std::size_t size>
auto ModbusRTU_Toyo::parseControllerInfos(Array<uint16_t, size> uiBuf) -> void
{
    controllerInformation.uiBatSoC = uiBuf.at(0);
    controllerInformation.fBatVolt = static_cast<float>(uiBuf.at(1) * 0.1F);
    controllerInformation.fBatCurr = static_cast<float>(uiBuf.at(2) * 0.01F);
    uint16_t uiTemp = uiBuf.at(3);
    controllerInformation.uiDevTemp = static_cast<uint16_t>(mainCore.util.byteSelect16(atUtilities::bytePos::HIGH_BYTE, uiTemp));
    controllerInformation.uiBatSurTemp = static_cast<uint16_t>(mainCore.util.byteSelect16(atUtilities::bytePos::LOW_BYTE, uiTemp));
    controllerInformation.fLoadVolt = static_cast<float>(uiBuf.at(4) * 0.1F);
    controllerInformation.fLoadCurr = static_cast<float>(uiBuf.at(5) * 0.01F);
    controllerInformation.uiLoadPwr = uiBuf.at(6);
    controllerInformation.fSolarVolt = static_cast<float>(uiBuf.at(7) * 0.1F);
    controllerInformation.fSolarCurr = static_cast<float>(uiBuf.at(8) * 0.01F);
    controllerInformation.uiSolarPwr = uiBuf.at(9);
    controllerInformation.fBatMinVoltToday = static_cast<float>(uiBuf.at(11) * 0.1F);
    controllerInformation.fBatMaxVoltToday = static_cast<float>(uiBuf.at(12) * 0.1F);
    controllerInformation.fMaxChargCurrToday = static_cast<float>(uiBuf.at(13) * 0.01F);
    controllerInformation.fMaxLoadCurrToday = static_cast<float>(uiBuf.at(14) * 0.01F);
    controllerInformation.uiMaxChargPwrToday = uiBuf.at(15);
    controllerInformation.uiMaxLoadPwrToday = uiBuf.at(16);
    controllerInformation.uiChargAhToday = uiBuf.at(17);
    controllerInformation.uiLoadAhToday = uiBuf.at(18);
    controllerInformation.uiPwrGenerationToday = uiBuf.at(19);
    controllerInformation.uiPwrConsumptionToday = uiBuf.at(20);
    controllerInformation.uiTotalDaysOfOperation = uiBuf.at(21);
    controllerInformation.uiTotalNoOverDischarges = uiBuf.at(22);
    controllerInformation.uiTotalNoFullCharges = uiBuf.at(23);
    controllerInformation.uiTotalAmpHoursCharged = ((uint32_t)uiBuf.at(24)) << 16 | uiBuf.at(25);
    controllerInformation.uiTotalAmpHoursDrawn = ((uint32_t)uiBuf.at(26) << 16) | uiBuf.at(27);
    controllerInformation.uiCumulativeKwhGeneration = ((uint32_t)uiBuf.at(28) << 16) | uiBuf.at(29);
    controllerInformation.uiCumulativeKwhDrawn = ((uint32_t)uiBuf.at(30) << 16) | uiBuf.at(31);
    controllerInformation.xLoadOutputActive = mainCore.util.bitManipulation(atUtilities::permission::READ, uiBuf.at(32), 15, false);
    controllerInformation.uiLoadBrightness = mainCore.util.selectMultipleBits16(static_cast<uint16_t>(mainCore.util.byteSelect16(atUtilities::bytePos::HIGH_BYTE, uiBuf.at(32))), 0, 6);
    controllerInformation.uiChargingState = mainCore.util.byteSelect16(atUtilities::bytePos::LOW_BYTE, uiBuf.at(32));
    controllerInformation.sChargingState = translateChargingState(controllerInformation.uiChargingState, uiLanguage);
    // TODO!
}
/**
 * @brief Read and Write to ModbusRTU
 *
 * @param uiReadInterval  How often should the controller be read in milliseconds
 */
auto ModbusRTU_Toyo::update(uint16_t uiReadInterval) -> void
{
    readContrInfo(1, 0x100, uiReadBuffer, debugMode::NONE);
    writeToStruct(description::CONTROLLER_INFOS);
}
/**
 * @brief Writing Holding Registers of Mppt Charger
 * !Function includes interceptions for preventing the insertion of wrong values
 *
 * @param uiDevice
 * @param targetVariable
 * @param uiValue
 * @param dLevel
 */
auto ModbusRTU_Toyo::writeContrInfo(uint16_t uiDevice,
                                    toyoWritableValues targetVariable,
                                    uint16_t uiValue,
                                    debugMode dLevel) -> void
{
    switch (targetVariable)
    {
        // switching on the load output (on = 1, off = 0)
    case toyoWritableValues::LOAD_OUTPUT:
        if (uiValue > 1 || uiValue < 0)
        {
            printDebug(debugSettings::PRINT_WRONG_VALUE_INPUT, "Load Output only allows 0 or 1", __func__);
            break;
        }
        else
        {
            if (!ModbusRTUClient.holdingRegisterWrite(uiDevice, 0x10A, uiValue))
            {
                if (dLevel == debugMode::ACTIVE)
                    printDebug(debugSettings::PRINT_WRITE_ERROR, 0, __func__);
            }
        }
        break;

    default:
        Serial.println("No valid Option for writeContrInfo");
        break;
    }
}

// ##############
// #  TOOLS #
// ##############

/**
 * @brief
 *
 * @param s
 * @return String
 */
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
        Serial.print("Failed to request while " + sFunctionName + " | Error Code : ");
        Serial.println(ModbusRTUClient.lastError());
        break;
    case debugSettings::PRINT_WRITE_ERROR:
        Serial.print("Failed to write while " + sFunctionName + " | Error Code : ");
        Serial.println(ModbusRTUClient.lastError());
        break;
    case debugSettings::PRINT_WRONG_VALUE_INPUT:
        Serial.println("Wrong value input in function" + sFunctionName + " | Note:  " + String(debugVariable));
        break;

    default:
        Serial.println("No debug setting selected!");
        break;
    }
}
/**
 * @brief translate the no. of the charging state to the corresponding string
 *
 * @param uiChargingState no. of the charging state
 * @param uiLang language no of ModbusRTU_Toyo::language
 * @return String
 */
auto ModbusRTU_Toyo::translateChargingState(uint8_t uiChargingState, uint8_t uiLang ) -> String
{
    String sResult;
    switch (uiLang)
    {
    case static_cast<uint8_t>(ModbusRTU_Toyo::language::EN) : 
        switch (uiChargingState)
        {
        case 0:
            sResult = "Charging deactivated";
            break;
        case 1:
            sResult = "Charging activated";
            break;
        case 2:
            sResult = "MPPT charging";
            break;
        case 3:
            sResult = "Equalizing charging";
            break;
        case 4:
            sResult = "Boost charging";
            break;
        case 5:
            sResult = "Floating charging";
            break;
        case 6:
            sResult = "Current limiting";
            break;
        }
        break;

    case static_cast<uint8_t>(ModbusRTU_Toyo::language::DE) :
        switch (uiChargingState)
        {
        case 0:
            sResult = "Laden deaktiviert";
            break;
        case 1:
            sResult = "Laden aktiviert";
            break;
        case 2:
            sResult = "MPPT Laden";
            break;
        case 3:
            sResult = "Ausgleichsladen";
            break;
        case 4:
            sResult = "Boost Laden";
            break;
        case 5:
            sResult = "Float Laden";
            break;
        case 6:
            sResult = "Strombegrenzung";
            break;
        }
        break;
    }
    return sResult;
}