#pragma once

#include <cstdint>

namespace solar::register_map {

// Zero-based Modbus TCP offsets shared by the Opta gateway and the IPC.
// Confirm address base, word order, and physical feedback wiring before commissioning.
inline constexpr std::uint16_t DigitalInputs = 0;
inline constexpr std::uint16_t AnalogInput1 = 1;
inline constexpr std::uint16_t BatteryVoltage = 2; // V x 100
inline constexpr std::uint16_t ChargingCurrent = 3; // A x 100, raw MPPT 0x0102 value
inline constexpr std::uint16_t PvPowerHighWord = 4; // W, high word
inline constexpr std::uint16_t PvPowerLowWord = 5; // W, low word
inline constexpr std::uint16_t MpptStatus = 6; // raw MPPT status word
inline constexpr std::uint16_t ContactorFeedback = 7; // auxiliary feedback, or commanded state
inline constexpr std::uint16_t FaultStatus = 8; // gateway fault bitfield
inline constexpr std::uint16_t MpptFaultHighWord = 9;
inline constexpr std::uint16_t MpptFaultLowWord = 10;
inline constexpr std::uint16_t GatewayStatus = 11;
inline constexpr std::uint16_t RtuErrorCount = 12;
inline constexpr std::uint16_t MpptSoc = 13;
inline constexpr std::uint16_t PvVoltage = 14;
inline constexpr std::uint16_t PvCurrent = 15;
inline constexpr std::uint16_t LoadVoltage = 16;
inline constexpr std::uint16_t LoadCurrent = 17;
inline constexpr std::uint16_t LoadPower = 18;
inline constexpr std::uint16_t MpptTemperatureRaw = 19;
inline constexpr std::uint16_t MpptRawRegisterBase = 20;
inline constexpr std::uint16_t MpptRawRegisterCount = 35;
inline constexpr std::uint16_t MpptSystemInformationRawBase = 55;
inline constexpr std::uint16_t MpptSystemInformationRawCount = 17;
inline constexpr std::uint16_t InputRegisterCount = 72;
inline constexpr std::uint16_t RequestedContactors = 100;
inline constexpr std::uint16_t IpcHeartbeat = 101;
inline constexpr std::uint16_t CommandSequence = 102;

// Requested/feedback contactor bitfield.
inline constexpr std::uint8_t MicroinverterAcContactorBit = 0;
inline constexpr std::uint8_t MicroinverterDcContactorBit = 1;
inline constexpr std::uint8_t MpptDcContactorBit = 2;
inline constexpr std::uint8_t IslandInverterDcContactorBit = 3;

} // namespace solar::register_map
