#pragma once

#include <chrono>
#include <cmath>
#include <optional>
#include <string_view>

namespace solar {

using Clock = std::chrono::steady_clock;

enum class OperatingMode {
    Auto,
    Manual,
    ForceMppt,
    ForceMicroinverter,
    Off,
};

enum class ManualTarget {
    Mppt,
    Microinverter,
    Off,
};

enum class PhysicalState {
    Unknown,
    AllOff,
    MpptActive,
    MicroinverterActive,
    SwitchingToAllOff,
    SwitchingToMppt,
    SwitchingToMicroinverter,
    Fault,
};

enum class Contactor {
    K1MicroinverterAc,
    K2MicroinverterDc,
    K3MpptDc,
    K4IslandInverterDc,
};

enum class FaultCode {
    None,
    InvalidFeedback,
    InterlockViolation,
    FeedbackTimeout,
    IslandInverterProtectionTimeout,
    InvalidTarget,
    InvalidProtectionSettings,
    StaleBatteryMeasurement,
    NotReconciled,
    Busy,
};

struct ContactorState {
    bool k1MicroinverterAcClosed{false};
    bool k2MicroinverterDcClosed{false};
    bool k3MpptDcClosed{false};
    bool k4IslandInverterDcClosed{false};

    friend bool operator==(const ContactorState&, const ContactorState&) = default;
};

struct ContactorFeedback {
    ContactorState state{};
    bool valid{false};
};

struct BatteryMeasurements {
    double voltage{0.0};
    double chargingCurrent{0.0};
    bool valid{false};
};

struct SolarSettings {
    std::optional<double> voltageLow{};
    std::optional<double> voltageHigh{};
    std::optional<double> islandInverterCutoffVoltage{};
    std::optional<double> islandInverterEnableVoltage{};
    std::chrono::milliseconds interStepDelay{1000};
    std::chrono::seconds voltageConfirmTime{30};
    std::chrono::seconds islandInverterVoltageConfirmTime{5};
    std::chrono::seconds minimumModeDuration{300};

    [[nodiscard]] bool thresholdsValid() const noexcept {
        return voltageLow.has_value() && voltageHigh.has_value()
            && std::isfinite(*voltageLow) && std::isfinite(*voltageHigh)
            && *voltageLow < *voltageHigh;
    }

    [[nodiscard]] bool islandInverterProtectionValid() const noexcept {
        return islandInverterCutoffVoltage.has_value()
            && islandInverterEnableVoltage.has_value()
            && std::isfinite(*islandInverterCutoffVoltage)
            && std::isfinite(*islandInverterEnableVoltage)
            && *islandInverterCutoffVoltage < *islandInverterEnableVoltage;
    }
};

inline std::optional<PhysicalState> physicalStateFromContactors(const ContactorState& state) noexcept {
    if (state.k2MicroinverterDcClosed && state.k3MpptDcClosed) {
        return std::nullopt;
    }
    if (!state.k1MicroinverterAcClosed && !state.k2MicroinverterDcClosed && !state.k3MpptDcClosed) {
        return PhysicalState::AllOff;
    }
    if (!state.k1MicroinverterAcClosed && !state.k2MicroinverterDcClosed && state.k3MpptDcClosed) {
        return PhysicalState::MpptActive;
    }
    if (state.k1MicroinverterAcClosed && state.k2MicroinverterDcClosed && !state.k3MpptDcClosed) {
        return PhysicalState::MicroinverterActive;
    }
    return PhysicalState::Unknown;
}

inline constexpr std::string_view toString(OperatingMode mode) noexcept {
    switch (mode) {
    case OperatingMode::Auto: return "AUTO";
    case OperatingMode::Manual: return "MANUAL";
    case OperatingMode::ForceMppt: return "FORCE_MPPT";
    case OperatingMode::ForceMicroinverter: return "FORCE_MICROINVERTER";
    case OperatingMode::Off: return "OFF";
    }
    return "UNKNOWN";
}

inline constexpr std::string_view toString(ManualTarget target) noexcept {
    switch (target) {
    case ManualTarget::Mppt: return "MPPT";
    case ManualTarget::Microinverter: return "MICROINVERTER";
    case ManualTarget::Off: return "OFF";
    }
    return "UNKNOWN";
}

inline constexpr std::string_view toString(PhysicalState state) noexcept {
    switch (state) {
    case PhysicalState::Unknown: return "UNKNOWN";
    case PhysicalState::AllOff: return "ALL_OFF";
    case PhysicalState::MpptActive: return "MPPT_ACTIVE";
    case PhysicalState::MicroinverterActive: return "MICROINVERTER_ACTIVE";
    case PhysicalState::SwitchingToAllOff: return "SWITCHING_TO_ALL_OFF";
    case PhysicalState::SwitchingToMppt: return "SWITCHING_TO_MPPT";
    case PhysicalState::SwitchingToMicroinverter: return "SWITCHING_TO_MICROINVERTER";
    case PhysicalState::Fault: return "FAULT";
    }
    return "UNKNOWN";
}

inline constexpr std::string_view toString(FaultCode fault) noexcept {
    switch (fault) {
    case FaultCode::None: return "NONE";
    case FaultCode::InvalidFeedback: return "INVALID_FEEDBACK";
    case FaultCode::InterlockViolation: return "INTERLOCK_VIOLATION";
    case FaultCode::FeedbackTimeout: return "FEEDBACK_TIMEOUT";
    case FaultCode::IslandInverterProtectionTimeout: return "ISLAND_INVERTER_PROTECTION_TIMEOUT";
    case FaultCode::InvalidTarget: return "INVALID_TARGET";
    case FaultCode::InvalidProtectionSettings: return "INVALID_PROTECTION_SETTINGS";
    case FaultCode::StaleBatteryMeasurement: return "STALE_BATTERY_MEASUREMENT";
    case FaultCode::NotReconciled: return "NOT_RECONCILED";
    case FaultCode::Busy: return "BUSY";
    }
    return "UNKNOWN";
}

} // namespace solar
