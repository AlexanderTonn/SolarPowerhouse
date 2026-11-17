#include "MockHardwareInterface.hpp"

void MockHardwareInterface::commandContactor(solar::Contactor contactor, bool closed) {
    commands_.push_back({contactor, closed});
}

solar::ContactorFeedback MockHardwareInterface::readContactorFeedback() const {
    return feedback_;
}

void MockHardwareInterface::setFeedback(solar::ContactorState state, bool valid) {
    feedback_ = {state, valid};
}
