#pragma once

#include "domain/IHardwareInterface.hpp"

#include <vector>

class MockHardwareInterface final : public solar::IHardwareInterface {
public:
    struct Command {
        solar::Contactor contactor;
        bool closed;
    };

    void commandContactor(solar::Contactor contactor, bool closed) override;
    [[nodiscard]] solar::ContactorFeedback readContactorFeedback() const override;

    void setFeedback(solar::ContactorState state, bool valid = true);
    [[nodiscard]] const std::vector<Command>& commands() const noexcept { return commands_; }
    void clearCommands() { commands_.clear(); }

private:
    solar::ContactorFeedback feedback_{};
    std::vector<Command> commands_{};
};
