#include "domain/BatteryMonitor.hpp"

namespace solar {

BatteryMonitor::BatteryMonitor(std::chrono::seconds maximumMeasurementAge)
    : maximumMeasurementAge_(maximumMeasurementAge) {}

void BatteryMonitor::update(const BatteryMeasurements& measurements, Clock::time_point now) {
    if (!measurements.valid || !std::isfinite(measurements.voltage)) {
        invalidate();
        return;
    }
    latest_ = measurements;
    updatedAt_ = now;
}

void BatteryMonitor::invalidate() {
    latest_.reset();
    belowLowSince_.reset();
    aboveHighSince_.reset();
}

std::optional<PhysicalState> BatteryMonitor::evaluateAutomaticTarget(
    PhysicalState currentState,
    const SolarSettings& settings,
    Clock::time_point now) {
    if (!settings.thresholdsValid() || !latest_.has_value()) {
        belowLowSince_.reset();
        aboveHighSince_.reset();
        return std::nullopt;
    }
    if (now - updatedAt_ > maximumMeasurementAge_) {
        belowLowSince_.reset();
        aboveHighSince_.reset();
        return std::nullopt;
    }

    const double voltage = latest_->voltage;
    // Battery below charge threshold: route PV to MPPT and charge battery.
    if (voltage <= *settings.chargeBelowVoltage) {
        aboveHighSince_.reset();
        if (!belowLowSince_.has_value()) {
            belowLowSince_ = now;
        }
        if (now - *belowLowSince_ >= settings.voltageConfirmTime
            && currentState != PhysicalState::MpptActive) {
            return PhysicalState::MpptActive;
        }
        return std::nullopt;
    }

    // Battery above full threshold: battery is considered full; route to
    // microinverter so charging stops.
    if (voltage >= *settings.batteryFullAboveVoltage) {
        belowLowSince_.reset();
        if (!aboveHighSince_.has_value()) {
            aboveHighSince_ = now;
        }
        if (now - *aboveHighSince_ >= settings.voltageConfirmTime
            && currentState != PhysicalState::MicroinverterActive) {
            return PhysicalState::MicroinverterActive;
        }
        return std::nullopt;
    }

    belowLowSince_.reset();
    aboveHighSince_.reset();
    return std::nullopt;
}

} // namespace solar
