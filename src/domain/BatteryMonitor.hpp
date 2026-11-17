#pragma once

#include "shared/SolarTypes.hpp"

#include <optional>

namespace solar {

class BatteryMonitor {
public:
    explicit BatteryMonitor(std::chrono::seconds maximumMeasurementAge = std::chrono::seconds{5});

    void update(const BatteryMeasurements& measurements, Clock::time_point now);
    void invalidate();

    [[nodiscard]] std::optional<PhysicalState> evaluateAutomaticTarget(
        PhysicalState currentState,
        const SolarSettings& settings,
        Clock::time_point now);

    [[nodiscard]] const std::optional<BatteryMeasurements>& latest() const noexcept { return latest_; }

private:
    std::chrono::seconds maximumMeasurementAge_;
    std::optional<BatteryMeasurements> latest_{};
    Clock::time_point updatedAt_{};
    std::optional<Clock::time_point> belowLowSince_{};
    std::optional<Clock::time_point> aboveHighSince_{};
};

} // namespace solar
