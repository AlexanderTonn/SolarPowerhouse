#include "TestSupport.hpp"

#include "infrastructure/OptaModbusTcp.hpp"

#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>

void runOptaModbusTcpTests() {
    using solar::infrastructure::MpptRegisterCount;
    using solar::infrastructure::SystemInformationRegisterCount;
    using solar::infrastructure::decodeToyoRenogy;

    const auto configPath = std::filesystem::temp_directory_path()
        / "solarpowerhouse-modbus-connection-test.conf";
    std::error_code filesystemError;
    std::filesystem::remove(configPath, filesystemError);
    {
        std::ofstream config(configPath);
        config << "host=10.20.30.40\n"
                  "subnet=255.255.255.0\n"
                  "gateway=10.20.30.1\n"
                  "port=1502\n"
                  "unit_id=7\n";
    }
    auto connection = solar::infrastructure::defaultModbusConnectionSettings();
    REQUIRE(solar::infrastructure::loadModbusConnectionSettings(
        configPath.string(), connection));
    REQUIRE(connection.host == "10.20.30.40");
    REQUIRE(connection.subnet == "255.255.255.0");
    REQUIRE(connection.gateway == "10.20.30.1");
    REQUIRE(connection.port == 1502);
    REQUIRE(connection.unitId == 7);
    REQUIRE(solar::infrastructure::validModbusConnectionSettings(connection));
    solar::infrastructure::OptaModbusTcpClient configuredClient(
        solar::infrastructure::ModbusMappingId::ToyoRenogy,
        connection);
    REQUIRE(configuredClient.connection().host == "10.20.30.40");
    REQUIRE(configuredClient.connection().unitId == 7);
    std::filesystem::remove(configPath, filesystemError);

    std::array<std::uint16_t, SystemInformationRegisterCount> systemInformation{};
    systemInformation[0] = 0x182D; // 24 V, 45 A charging
    systemInformation[1] = 0x1E01; // 30 A discharging, inverter type
    systemInformation[2] = 0x544F; // TO
    systemInformation[3] = 0x594F; // YO
    systemInformation[10] = 0x312E; // 1.
    systemInformation[11] = 0x3220; // 2
    systemInformation[12] = 0x4831; // H1
    systemInformation[13] = 0x2E30; // .0
    systemInformation[14] = 0x534E; // SN
    systemInformation[15] = 0x3031; // 01
    systemInformation[16] = 1;

    std::array<std::uint16_t, MpptRegisterCount> registers{};
    registers[0] = 82;
    registers[1] = 128;
    registers[2] = 235;
    registers[3] = 0x0C86; // controller +12 C, battery -6 C
    registers[4] = 126;
    registers[5] = 150;
    registers[6] = 240;
    registers[7] = 144;
    registers[8] = 150;
    registers[9] = 216;
    registers[11] = 112;
    registers[12] = 132;
    registers[13] = 216;
    registers[14] = 90;
    registers[15] = 500;
    registers[16] = 400;
    registers[17] = 1544;
    registers[18] = 2064;
    registers[19] = 1250;
    registers[20] = 840;
    registers[21] = 8;
    registers[22] = 1;
    registers[23] = 6;
    registers[24] = 0x0001;
    registers[25] = 0x0203;
    registers[26] = 0x0004;
    registers[27] = 0x0506;
    registers[28] = 0x0007;
    registers[29] = 0x0809;
    registers[30] = 0x000A;
    registers[31] = 0x0B0C;
    registers[32] = 0xE402; // load on, 100 percent, MPPT charging
    registers[33] = 0x1234;
    registers[34] = 0x5678;

    const auto result = decodeToyoRenogy(systemInformation, registers);
    REQUIRE(result.maximumSystemVoltage == 24);
    REQUIRE(result.ratedChargingCurrent == 45);
    REQUIRE(result.ratedDischargingCurrent == 30);
    REQUIRE(result.productType == 1);
    REQUIRE(result.productModel == "TOYO");
    REQUIRE(result.softwareVersion == "1.2");
    REQUIRE(result.hardwareVersion == "H1.0");
    REQUIRE(result.serialNumber == "SN01");
    REQUIRE(result.deviceAddress == 1);
    REQUIRE(result.stateOfChargePercent == 82);
    REQUIRE(std::abs(result.batteryVoltage - 12.8) < 0.001);
    REQUIRE(std::abs(result.chargingCurrent - 2.35) < 0.001);
    REQUIRE(std::abs(result.controllerTemperature - 12.0) < 0.001);
    REQUIRE(std::abs(result.batteryTemperature + 6.0) < 0.001);
    REQUIRE(result.dailyChargingAmpHours == 1544);
    REQUIRE(result.totalChargingAmpHours == 0x00010203u);
    REQUIRE(std::abs(result.cumulativeEnergyGenerationKwh - 46.0809) < 0.001);
    REQUIRE(result.streetLightOn);
    REQUIRE(result.streetLightBrightnessPercent == 100);
    REQUIRE(result.chargingState == 2);
    REQUIRE(result.controllerFaults == 0x12345678u);
}
