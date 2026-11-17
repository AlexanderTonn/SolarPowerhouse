#pragma once

#include "domain/BatteryMonitor.hpp"

#include <optional>

namespace solar {

class EnergyManager {
public:
    explicit EnergyManager(BatteryMonitor& batteryMonitor);

    void recordStableState(PhysicalState state, Clock::time_point now);

    [[nodiscard]] std::optional<PhysicalState> selectTarget(
        OperatingMode mode,
        PhysicalState currentState,
        const SolarSettings& settings,
        Clock::time_point now);

    [[nodiscard]] std::optional<PhysicalState> selectTarget(
        OperatingMode mode,
        ManualTarget manualTarget,
        PhysicalState currentState,
        const SolarSettings& settings,
        Clock::time_point now);

private:
    BatteryMonitor& batteryMonitor_;
    std::optional<Clock::time_point> stableSince_{};
};

} // namespace solar
