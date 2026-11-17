#pragma once

#include <cstdint>

namespace solar {
namespace opta {
namespace register_map {

// Modbus TCP input registers: Opta -> IPC.
constexpr std::uint16_t DigitalInputs = 0;
constexpr std::uint16_t AnalogInput1 = 1;
constexpr std::uint16_t BatteryVoltage = 2; // V x 100
constexpr std::uint16_t ChargingCurrent = 3; // A x 100
constexpr std::uint16_t PvPowerHighWord = 4;
constexpr std::uint16_t PvPowerLowWord = 5;
constexpr std::uint16_t MpptStatus = 6;
constexpr std::uint16_t ContactorFeedback = 7;
constexpr std::uint16_t FaultStatus = 8;
constexpr std::uint16_t MpptFaultHighWord = 9;
constexpr std::uint16_t MpptFaultLowWord = 10;
constexpr std::uint16_t GatewayStatus = 11;
constexpr std::uint16_t RtuErrorCount = 12;
constexpr std::uint16_t MpptSoc = 13;
constexpr std::uint16_t PvVoltage = 14;
constexpr std::uint16_t PvCurrent = 15;
constexpr std::uint16_t LoadVoltage = 16;
constexpr std::uint16_t LoadCurrent = 17;
constexpr std::uint16_t LoadPower = 18;
constexpr std::uint16_t MpptTemperatureRaw = 19;
constexpr std::uint16_t MpptRawRegisterBase = 20;
constexpr std::uint16_t MpptRawRegisterCount = 35;
constexpr std::uint16_t MpptSystemInformationRawBase = 55;
constexpr std::uint16_t MpptSystemInformationRawCount = 17;
constexpr std::uint16_t InputRegisterCount = 72;

static_assert(MpptRawRegisterBase + MpptRawRegisterCount
                  == MpptSystemInformationRawBase,
              "MPPT raw register blocks must not overlap");
static_assert(MpptSystemInformationRawBase + MpptSystemInformationRawCount
                  == InputRegisterCount,
              "Input register count must cover all published values");

// Modbus TCP holding registers: IPC -> Opta.
constexpr std::uint16_t RequestedContactors = 100;
constexpr std::uint16_t IpcHeartbeat = 101;
constexpr std::uint16_t CommandSequence = 102;
constexpr std::uint16_t HoldingRegisterStart = RequestedContactors;
constexpr std::uint16_t HoldingRegisterCount = 3;

constexpr std::uint16_t K1Bit = 1u << 0;
constexpr std::uint16_t K2Bit = 1u << 1;
constexpr std::uint16_t K3Bit = 1u << 2;
constexpr std::uint16_t K4Bit = 1u << 3;
constexpr std::uint16_t ContactorMask = K1Bit | K2Bit | K3Bit | K4Bit;

} // namespace register_map
} // namespace opta
} // namespace solar

namespace solar {
namespace opta {
namespace fault_bits {

constexpr std::uint16_t MpptCommunication = 1u << 0;
constexpr std::uint16_t InvalidContactorCommand = 1u << 1;
constexpr std::uint16_t InterlockViolation = 1u << 2;
constexpr std::uint16_t IpcHeartbeatExpired = 1u << 3;
constexpr std::uint16_t MpptReportedFault = 1u << 4;

} // namespace fault_bits
} // namespace opta
} // namespace solar
