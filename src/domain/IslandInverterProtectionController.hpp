#pragma once

#include "domain/EventManager.hpp"
#include "domain/IHardwareInterface.hpp"

#include <optional>

namespace solar {

enum class IslandInverterProtectionState {
    Unknown,
    Enabled,
    CutOff,
    SwitchingOff,
    SwitchingOn,
    Fault,
};

class IslandInverterProtectionController {
public:
    struct Config {
        std::chrono::seconds maximumMeasurementAge{5};
        std::chrono::seconds feedbackTimeout{3};
    };

    explicit IslandInverterProtectionController(IHardwareInterface& hardware);
    IslandInverterProtectionController(IHardwareInterface& hardware, Config config);

    // Observe actual K4 feedback. Never assumes a startup output state.
    void reconcile();
    void setEventManager(EventManager* eventManager) noexcept { eventManager_ = eventManager; }
    void updateBatteryMeasurement(const BatteryMeasurements& measurement, Clock::time_point now);
    void evaluate(const SolarSettings& settings, Clock::time_point now);
    void tick(Clock::time_point now);
    bool acknowledgeFault();

    [[nodiscard]] IslandInverterProtectionState state() const noexcept { return state_; }
    [[nodiscard]] FaultCode fault() const noexcept { return fault_; }
    [[nodiscard]] bool busy() const noexcept { return waitingForFeedback_; }

private:
    void fail(FaultCode fault);
    void startTransition(bool enable, Clock::time_point now);

    IHardwareInterface& hardware_;
    Config config_;
    EventManager* eventManager_{nullptr};
    IslandInverterProtectionState state_{IslandInverterProtectionState::Unknown};
    FaultCode fault_{FaultCode::None};
    std::optional<BatteryMeasurements> latestMeasurement_{};
    Clock::time_point measurementUpdatedAt_{};
    std::optional<Clock::time_point> belowCutoffSince_{};
    std::optional<Clock::time_point> aboveEnableSince_{};
    Clock::time_point feedbackDeadline_{};
    bool expectedClosed_{false};
    bool waitingForFeedback_{false};
};

} // namespace solar
