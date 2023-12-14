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
auto ModbusRTU_Toyo::init(const uint16_t baud) -> bool
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
                                   std::array<uint16_t, size> &uiData,
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
auto ModbusRTU_Toyo::parseControllerInfos(std::array<uint16_t, size> uiBuf) -> void
{
    controllerInformation.uiBatSoC = uiBuf.at(0);
    controllerInformation.fBatVolt = static_cast<float>(uiBuf.at(1) * 0.1F);
    controllerInformation.fBatCurr = static_cast<float>(uiBuf.at(2) * 0.01F);
    uint16_t uiTemp = uiBuf.at(3);
    controllerInformation.uiDevTemp = static_cast<uint16_t>(selectByte(bytePos::HIGH_BYTE, uiTemp));
    controllerInformation.uiBatSurTemp = static_cast<uint16_t>(selectByte(bytePos::LOW_BYTE, uiTemp));
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
    controllerInformation.uiPwrGenerationToday = uiBuf.at(19)/10000;
    controllerInformation.uiPwrConsumptionToday = uiBuf.at(20)/10000;
    controllerInformation.uiTotalDaysOfOperation = uiBuf.at(21);
    controllerInformation.uiTotalNoOverDischarges = uiBuf.at(22);
    controllerInformation.uiTotalAmpHoursCharged = uiBuf.at(23);
    controllerInformation.uiTotalAmpHoursCharged += uiBuf.at(24) << 16; // adding high byte
    controllerInformation.uiTotalAmpHoursDrawn = uiBuf.at(25);
    controllerInformation.uiTotalAmpHoursDrawn += uiBuf.at(26) << 16; // adding high byte
    // TODO!
}
/**
 * @brief Read and Write to ModbusRTU
 *
 * @param uiReadInterval  How often should the controller be read in milliseconds
 */
auto ModbusRTU_Toyo::update(uint16_t uiReadInterval) -> void
{
    Serial.println("ModbusRTU_Toyo::" + String(__func__));
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
 * @brief Select the high or low byte of a 16 bit value
 * ! Low Byte: 0x00FF
 * ! High Byte: 0xFF00
 * @param pos
 * @param uiInput
 * @return uint8_t
 */
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