#include "domain/IslandInverterProtectionController.hpp"

#include <cmath>

namespace solar {

IslandInverterProtectionController::IslandInverterProtectionController(
    IHardwareInterface& hardware)
    : IslandInverterProtectionController(hardware, Config{}) {}

IslandInverterProtectionController::IslandInverterProtectionController(
    IHardwareInterface& hardware,
    Config config)
    : hardware_(hardware), config_(config) {}

void IslandInverterProtectionController::reconcile() {
    const auto feedback = hardware_.readContactorFeedback();
    if (!feedback.valid) {
        fail(FaultCode::InvalidFeedback);
        return;
    }
    state_ = feedback.state.k4IslandInverterDcClosed
        ? IslandInverterProtectionState::Enabled
        : IslandInverterProtectionState::CutOff;
    fault_ = FaultCode::None;
    waitingForFeedback_ = false;
}

void IslandInverterProtectionController::updateBatteryMeasurement(
    const BatteryMeasurements& measurement,
    Clock::time_point now) {
    if (!measurement.valid || !std::isfinite(measurement.voltage)) {
        latestMeasurement_.reset();
        belowCutoffSince_.reset();
        aboveEnableSince_.reset();
        return;
    }
    latestMeasurement_ = measurement;
    measurementUpdatedAt_ = now;
}

void IslandInverterProtectionController::evaluate(
    const SolarSettings& settings,
    Clock::time_point now) {
    if (fault_ != FaultCode::None || waitingForFeedback_) {
        return;
    }
    if (!settings.islandInverterProtectionValid() || !latestMeasurement_.has_value()) {
        belowCutoffSince_.reset();
        aboveEnableSince_.reset();
        return;
    }
    if (now - measurementUpdatedAt_ > config_.maximumMeasurementAge) {
        belowCutoffSince_.reset();
        aboveEnableSince_.reset();
        return;
    }

    const auto voltage = latestMeasurement_->voltage;
    if (state_ == IslandInverterProtectionState::Enabled) {
        aboveEnableSince_.reset();
        if (voltage <= *settings.islandInverterCutoffVoltage) {
            if (!belowCutoffSince_.has_value()) {
                belowCutoffSince_ = now;
            }
            if (now - *belowCutoffSince_ >= settings.islandInverterVoltageConfirmTime) {
                startTransition(false, now);
            }
        } else {
            belowCutoffSince_.reset();
        }
        return;
    }

    if (state_ == IslandInverterProtectionState::CutOff) {
        belowCutoffSince_.reset();
        if (voltage >= *settings.islandInverterEnableVoltage) {
            if (!aboveEnableSince_.has_value()) {
                aboveEnableSince_ = now;
            }
            if (now - *aboveEnableSince_ >= settings.islandInverterVoltageConfirmTime) {
                startTransition(true, now);
            }
        } else {
            aboveEnableSince_.reset();
        }
    }
}

void IslandInverterProtectionController::tick(Clock::time_point now) {
    if (fault_ != FaultCode::None || !waitingForFeedback_) {
        return;
    }

    const auto feedback = hardware_.readContactorFeedback();
    if (feedback.valid && feedback.state.k4IslandInverterDcClosed == expectedClosed_) {
        state_ = expectedClosed_
            ? IslandInverterProtectionState::Enabled
            : IslandInverterProtectionState::CutOff;
        waitingForFeedback_ = false;
        belowCutoffSince_.reset();
        aboveEnableSince_.reset();
        return;
    }
    if (now >= feedbackDeadline_) {
        fail(FaultCode::IslandInverterProtectionTimeout);
    }
}

bool IslandInverterProtectionController::acknowledgeFault() {
    if (fault_ == FaultCode::None) {
        return true;
    }
    const auto feedback = hardware_.readContactorFeedback();
    if (!feedback.valid) {
        return false;
    }
    state_ = feedback.state.k4IslandInverterDcClosed
        ? IslandInverterProtectionState::Enabled
        : IslandInverterProtectionState::CutOff;
    fault_ = FaultCode::None;
    waitingForFeedback_ = false;
    return true;
}

void IslandInverterProtectionController::fail(FaultCode fault) {
    if (eventManager_) {
        eventManager_->record(
            EventType::FaultRaised,
            EventSeverity::Critical,
            "IslandInverterProtection",
            "Protection fault: " + std::string(toString(fault)));
    }
    fault_ = fault;
    state_ = IslandInverterProtectionState::Fault;
    waitingForFeedback_ = false;
}

void IslandInverterProtectionController::startTransition(bool enable, Clock::time_point now) {
    const auto feedback = hardware_.readContactorFeedback();
    if (!feedback.valid) {
        fail(FaultCode::InvalidFeedback);
        return;
    }
    expectedClosed_ = enable;
    if (eventManager_) {
        eventManager_->record(
            enable ? EventType::IslandInverterProtectionRestored : EventType::IslandInverterProtectionCutoff,
            enable ? EventSeverity::Info : EventSeverity::Warning,
            "IslandInverterProtection",
            enable ? "K4 island inverter DC contactor enable requested"
                   : "K4 island inverter DC contactor cut-off requested");
    }
    hardware_.commandContactor(Contactor::K4IslandInverterDc, enable);
    state_ = enable
        ? IslandInverterProtectionState::SwitchingOn
        : IslandInverterProtectionState::SwitchingOff;
    feedbackDeadline_ = now + config_.feedbackTimeout;
    waitingForFeedback_ = true;
}

} // namespace solar
