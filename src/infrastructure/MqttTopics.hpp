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
inline constexpr std::string_view VoltageLowCommand = "solarpowerhouse/v1/command/voltage_low";
inline constexpr std::string_view ConfirmedVoltageLow = "solarpowerhouse/v1/config/voltage_low";
inline constexpr std::string_view VoltageHighCommand = "solarpowerhouse/v1/command/voltage_high";
inline constexpr std::string_view ConfirmedVoltageHigh = "solarpowerhouse/v1/config/voltage_high";

} // namespace solar::mqtt_topics
