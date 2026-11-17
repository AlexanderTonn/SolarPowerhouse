#pragma once

#include "shared/SolarTypes.hpp"

namespace solar {

class IHardwareInterface {
public:
    virtual ~IHardwareInterface() = default;

    // Command exactly one output change. Feedback must confirm the resulting state.
    virtual void commandContactor(Contactor contactor, bool closed) = 0;
    [[nodiscard]] virtual ContactorFeedback readContactorFeedback() const = 0;
};

} // namespace solar
