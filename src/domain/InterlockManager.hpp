#pragma once

#include "shared/SolarTypes.hpp"

#include <string>

namespace solar {

struct InterlockDecision {
    bool allowed{false};
    FaultCode fault{FaultCode::None};
    std::string reason{};
};

class InterlockManager {
public:
    [[nodiscard]] InterlockDecision validate(
        const ContactorFeedback& feedback,
        const ContactorState& requested) const;
};

} // namespace solar
