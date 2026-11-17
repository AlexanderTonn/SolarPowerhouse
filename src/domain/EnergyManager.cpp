#include "domain/EnergyManager.hpp"

namespace solar {

EnergyManager::EnergyManager(BatteryMonitor& batteryMonitor)
    : batteryMonitor_(batteryMonitor) {}

void EnergyManager::recordStableState(PhysicalState state, Clock::time_point now) {
    if (state == PhysicalState::MpptActive || state == PhysicalState::MicroinverterActive) {
        stableSince_ = now;
    } else {
        stableSince_.reset();
    }
}

std::optional<PhysicalState> EnergyManager::selectTarget(
    OperatingMode mode,
    PhysicalState currentState,
    const SolarSettings& settings,
    Clock::time_point now) {
    return selectTarget(mode, ManualTarget::Mppt, currentState, settings, now);
}

std::optional<PhysicalState> EnergyManager::selectTarget(
    OperatingMode mode,
    ManualTarget manualTarget,
    PhysicalState currentState,
    const SolarSettings& settings,
    Clock::time_point now) {
    switch (mode) {
    case OperatingMode::ForceMppt:
        return PhysicalState::MpptActive;
    case OperatingMode::ForceMicroinverter:
        return PhysicalState::MicroinverterActive;
    case OperatingMode::Off:
        return PhysicalState::AllOff;
    case OperatingMode::Manual:
        switch (manualTarget) {
        case ManualTarget::Mppt: return PhysicalState::MpptActive;
        case ManualTarget::Microinverter: return PhysicalState::MicroinverterActive;
        case ManualTarget::Off: return PhysicalState::AllOff;
        }
    case OperatingMode::Auto:
        break;
    }

    const auto target = batteryMonitor_.evaluateAutomaticTarget(currentState, settings, now);
    if (!target.has_value() || !stableSince_.has_value()) {
        return target;
    }
    if (now - *stableSince_ < settings.minimumModeDuration) {
        return std::nullopt;
    }
    return target;
}

} // namespace solar
