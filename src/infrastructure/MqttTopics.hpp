#pragma once

#include <string_view>

namespace solar::mqtt_topics {

inline constexpr std::string_view Prefix = "solarpowerhouse/v1";
inline constexpr std::string_view Availability = "solarpowerhouse/v1/availability";
inline constexpr std::string_view State = "solarpowerhouse/v1/state";
inline constexpr std::string_view Config = "solarpowerhouse/v1/config";
inline constexpr std::string_view Command = "solarpowerhouse/v1/command";
inline constexpr std::string_view Event = "solarpowerhouse/v1/event";

inline constexpr std::string_view OperatingModeCommand = "solarpowerhouse/v1/command/operating_mode";
inline constexpr std::string_view ConfirmedOperatingMode = "solarpowerhouse/v1/state/operating_mode";
// Wire names remain voltage_low/high for protocol compatibility. Their
// application semantics are charge-below and battery-full-above.
inline constexpr std::string_view ChargeBelowVoltageCommand =
    "solarpowerhouse/v1/command/voltage_low";
inline constexpr std::string_view ConfirmedChargeBelowVoltage =
    "solarpowerhouse/v1/config/voltage_low";
inline constexpr std::string_view BatteryFullAboveVoltageCommand =
    "solarpowerhouse/v1/command/voltage_high";
inline constexpr std::string_view ConfirmedBatteryFullAboveVoltage =
    "solarpowerhouse/v1/config/voltage_high";

// Backward-compatible C++ names for existing integrations.
inline constexpr std::string_view VoltageLowCommand = ChargeBelowVoltageCommand;
inline constexpr std::string_view ConfirmedVoltageLow = ConfirmedChargeBelowVoltage;
inline constexpr std::string_view VoltageHighCommand = BatteryFullAboveVoltageCommand;
inline constexpr std::string_view ConfirmedVoltageHigh = ConfirmedBatteryFullAboveVoltage;

} // namespace solar::mqtt_topics
