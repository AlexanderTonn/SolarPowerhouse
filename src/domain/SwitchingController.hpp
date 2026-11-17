#pragma once

#include "domain/EventManager.hpp"
#include "domain/IHardwareInterface.hpp"
#include "domain/InterlockManager.hpp"

#include <optional>
#include <vector>

namespace solar {

class SwitchingController {
public:
    struct Config {
        std::chrono::milliseconds interStepDelay{1000};
        std::chrono::milliseconds feedbackTimeout{3000};
    };

    SwitchingController(IHardwareInterface& hardware, InterlockManager& interlocks);
    SwitchingController(IHardwareInterface& hardware, InterlockManager& interlocks, Config config);

    // Startup reconciliation only observes feedback. It never replays old commands.
    void reconcile(Clock::time_point now);
    void setEventManager(EventManager* eventManager) noexcept { eventManager_ = eventManager; }
    bool request(PhysicalState target, Clock::time_point now);
    void tick(Clock::time_point now);
    bool acknowledgeFault(Clock::time_point now);

    [[nodiscard]] PhysicalState state() const noexcept { return state_; }
    [[nodiscard]] FaultCode fault() const noexcept { return fault_; }
    [[nodiscard]] bool busy() const noexcept { return waitingForFeedback_ || waitingForDelay_; }

private:
    struct Step {
        Contactor contactor;
        bool closed;
        ContactorState expectedState;
    };

    void fail(FaultCode fault);
    void issueNextStep(Clock::time_point now);
    [[nodiscard]] std::vector<Step> buildSteps(const ContactorState& current, PhysicalState target) const;

    IHardwareInterface& hardware_;
    InterlockManager& interlocks_;
    EventManager* eventManager_{nullptr};
    Config config_;
    PhysicalState state_{PhysicalState::Unknown};
    PhysicalState target_{PhysicalState::Unknown};
    FaultCode fault_{FaultCode::None};
    std::vector<Step> steps_{};
    std::size_t stepIndex_{0};
    ContactorState expectedState_{};
    Clock::time_point feedbackDeadline_{};
    Clock::time_point nextActionAt_{};
    bool waitingForFeedback_{false};
    bool waitingForDelay_{false};
};

} // namespace solar
