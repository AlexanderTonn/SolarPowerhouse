#include "domain/SwitchingController.hpp"

#include <algorithm>

namespace solar {

namespace {

ContactorState targetContactors(PhysicalState target) {
    switch (target) {
    case PhysicalState::AllOff:
        return {};
    case PhysicalState::MpptActive:
        return {false, false, true};
    case PhysicalState::MicroinverterActive:
        return {true, true, false};
    default:
        return {};
    }
}

void setContactor(ContactorState& state, Contactor contactor, bool closed) {
    switch (contactor) {
    case Contactor::K1MicroinverterAc: state.k1MicroinverterAcClosed = closed; break;
    case Contactor::K2MicroinverterDc: state.k2MicroinverterDcClosed = closed; break;
    case Contactor::K3MpptDc: state.k3MpptDcClosed = closed; break;
    case Contactor::K4IslandInverterDc: state.k4IslandInverterDcClosed = closed; break;
    }
}

bool contactorValue(const ContactorState& state, Contactor contactor) {
    switch (contactor) {
    case Contactor::K1MicroinverterAc: return state.k1MicroinverterAcClosed;
    case Contactor::K2MicroinverterDc: return state.k2MicroinverterDcClosed;
    case Contactor::K3MpptDc: return state.k3MpptDcClosed;
    case Contactor::K4IslandInverterDc: return state.k4IslandInverterDcClosed;
    }
    return false;
}

} // namespace

SwitchingController::SwitchingController(
    IHardwareInterface& hardware,
    InterlockManager& interlocks)
    : SwitchingController(hardware, interlocks, Config{}) {}

SwitchingController::SwitchingController(
    IHardwareInterface& hardware,
    InterlockManager& interlocks,
    Config config)
    : hardware_(hardware), interlocks_(interlocks), config_(config) {}

void SwitchingController::reconcile(Clock::time_point /*now*/) {
    const auto feedback = hardware_.readContactorFeedback();
    const auto physicalState = feedback.valid ? physicalStateFromContactors(feedback.state) : std::nullopt;
    if (!physicalState.has_value()) {
        fail(feedback.valid ? FaultCode::InterlockViolation : FaultCode::InvalidFeedback);
        return;
    }
    state_ = *physicalState;
    target_ = state_;
    fault_ = FaultCode::None;
    steps_.clear();
    waitingForFeedback_ = false;
    waitingForDelay_ = false;
}

bool SwitchingController::request(PhysicalState target, Clock::time_point now) {
    if (fault_ != FaultCode::None) {
        return false;
    }
    if (busy()) {
        fault_ = FaultCode::Busy;
        return false;
    }
    if (target != PhysicalState::AllOff
        && target != PhysicalState::MpptActive
        && target != PhysicalState::MicroinverterActive) {
        fault_ = FaultCode::InvalidTarget;
        state_ = PhysicalState::Fault;
        return false;
    }
    if (state_ == PhysicalState::Unknown || state_ == PhysicalState::Fault) {
        fault_ = FaultCode::NotReconciled;
        state_ = PhysicalState::Fault;
        return false;
    }
    const auto feedback = hardware_.readContactorFeedback();
    if (!feedback.valid || physicalStateFromContactors(feedback.state) != state_) {
        fail(FaultCode::InvalidFeedback);
        return false;
    }
    if (state_ == target) {
        if (eventManager_) {
            eventManager_->record(
                EventType::SwitchingRequested,
                EventSeverity::Info,
                "SwitchingController",
                "Requested physical state already active: " + std::string(toString(target)));
        }
        return true;
    }

    if (eventManager_) {
        eventManager_->record(
            EventType::SwitchingRequested,
            EventSeverity::Info,
            "SwitchingController",
            "Requested physical state: " + std::string(toString(target)));
    }
    target_ = target;
    expectedState_ = feedback.state;
    steps_ = buildSteps(feedback.state, target);
    stepIndex_ = 0;
    state_ = target == PhysicalState::MpptActive
        ? PhysicalState::SwitchingToMppt
        : (target == PhysicalState::AllOff
            ? PhysicalState::SwitchingToAllOff
            : PhysicalState::SwitchingToMicroinverter);
    issueNextStep(now);
    return fault_ == FaultCode::None;
}

void SwitchingController::tick(Clock::time_point now) {
    if (fault_ != FaultCode::None || (!waitingForFeedback_ && !waitingForDelay_)) {
        return;
    }

    const auto feedback = hardware_.readContactorFeedback();
    const auto interlock = interlocks_.validate(feedback, expectedState_);
    if (!interlock.allowed) {
        fail(interlock.fault);
        return;
    }

    if (waitingForFeedback_) {
        if (feedback.valid && feedback.state == expectedState_) {
            waitingForFeedback_ = false;
            ++stepIndex_;
            if (stepIndex_ == steps_.size()) {
                state_ = target_;
                target_ = state_;
                steps_.clear();
                if (eventManager_ && state_ == PhysicalState::AllOff) {
                    eventManager_->record(
                        EventType::AllContactorsDeenergized,
                        EventSeverity::Warning,
                        "SwitchingController",
                        "All contactors de-energized");
                }
                return;
            }
            waitingForDelay_ = true;
            nextActionAt_ = now + config_.interStepDelay;
        } else if (now >= feedbackDeadline_) {
            fail(FaultCode::FeedbackTimeout);
        }
        return;
    }

    if (waitingForDelay_ && now >= nextActionAt_) {
        waitingForDelay_ = false;
        issueNextStep(now);
    }
}

bool SwitchingController::acknowledgeFault(Clock::time_point /*now*/) {
    if (fault_ == FaultCode::None) {
        return true;
    }
    const auto feedback = hardware_.readContactorFeedback();
    const auto physicalState = feedback.valid ? physicalStateFromContactors(feedback.state) : std::nullopt;
    if (!physicalState.has_value()) {
        return false;
    }
    state_ = *physicalState;
    target_ = state_;
    fault_ = FaultCode::None;
    steps_.clear();
    waitingForFeedback_ = false;
    waitingForDelay_ = false;
    return true;
}

void SwitchingController::fail(FaultCode fault) {
    if (eventManager_) {
        eventManager_->record(
            EventType::FaultRaised,
            EventSeverity::Critical,
            "SwitchingController",
            "Switching fault: " + std::string(toString(fault)));
    }
    fault_ = fault;
    state_ = PhysicalState::Fault;
    target_ = PhysicalState::Fault;
    steps_.clear();
    waitingForFeedback_ = false;
    waitingForDelay_ = false;
}

void SwitchingController::issueNextStep(Clock::time_point now) {
    if (stepIndex_ >= steps_.size()) {
        return;
    }
    const auto& step = steps_.at(stepIndex_);
    const auto feedback = hardware_.readContactorFeedback();
    const auto interlock = interlocks_.validate(feedback, step.expectedState);
    if (!interlock.allowed) {
        fail(interlock.fault);
        return;
    }
    expectedState_ = step.expectedState;
    hardware_.commandContactor(step.contactor, step.closed);
    waitingForFeedback_ = true;
    feedbackDeadline_ = now + config_.feedbackTimeout;
}

std::vector<SwitchingController::Step> SwitchingController::buildSteps(
    const ContactorState& current,
    PhysicalState target) const {
    auto desired = targetContactors(target);
    if (target != PhysicalState::AllOff) {
        // K4 belongs to island-inverter protection and stays unchanged during PV routing.
        desired.k4IslandInverterDcClosed = current.k4IslandInverterDcClosed;
    }
    std::vector<Step> result;
    ContactorState working = current;

    // Open energized paths first. This preserves the requested break-before-make order.
    const Contactor openingOrder[] = {
        Contactor::K1MicroinverterAc,
        Contactor::K2MicroinverterDc,
        Contactor::K3MpptDc,
        Contactor::K4IslandInverterDc,
    };
    for (const auto contactor : openingOrder) {
        if (contactorValue(working, contactor) && !contactorValue(desired, contactor)) {
            setContactor(working, contactor, false);
            result.push_back({contactor, false, working});
        }
    }

    const Contactor closingOrder[] = {
        Contactor::K3MpptDc,
        Contactor::K2MicroinverterDc,
        Contactor::K1MicroinverterAc,
    };
    for (const auto contactor : closingOrder) {
        if (!contactorValue(working, contactor) && contactorValue(desired, contactor)) {
            setContactor(working, contactor, true);
            result.push_back({contactor, true, working});
        }
    }
    return result;
}

} // namespace solar
