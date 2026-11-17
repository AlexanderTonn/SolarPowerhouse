#include "domain/InterlockManager.hpp"

namespace solar {

InterlockDecision InterlockManager::validate(
    const ContactorFeedback& feedback,
    const ContactorState& requested) const {
    if (feedback.valid && feedback.state.k2MicroinverterDcClosed && feedback.state.k3MpptDcClosed) {
        return {false, FaultCode::InterlockViolation, "Feedback reports K2 and K3 closed"};
    }
    if (requested.k2MicroinverterDcClosed && requested.k3MpptDcClosed) {
        return {false, FaultCode::InterlockViolation, "Requested state closes K2 and K3"};
    }
    return {true, FaultCode::None, {}};
}

} // namespace solar
