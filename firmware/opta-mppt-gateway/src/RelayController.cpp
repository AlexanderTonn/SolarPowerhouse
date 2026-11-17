#include "RelayController.hpp"

#include "ModbusRegisterMap.hpp"

namespace solar {
namespace opta {

RelayController::RelayController(const GatewayConfig& config)
    : config_(config) {}

void RelayController::begin() {
    for (const auto relayPin : config_.relayPins) {
        pinMode(relayPin, OUTPUT);
    }
    allOff();

    for (const auto inputPin : config_.digitalInputPins) {
        pinMode(inputPin, INPUT);
    }
}

void RelayController::allOff() {
    for (const auto relayPin : config_.relayPins) {
        digitalWrite(relayPin, LOW);
    }
    appliedBits_ = 0;
}

bool RelayController::applyCommand(std::uint16_t command) {
    if (!isValidCommand(command)) {
        allOff();
        return false;
    }

    for (std::uint8_t index = 0; index < 4; ++index) {
        const bool closed = (command & (1u << index)) != 0;
        digitalWrite(config_.relayPins[index], closed ? HIGH : LOW);
    }
    appliedBits_ = command;
    return true;
}

std::uint16_t RelayController::feedbackBits() const {
    if (!config_.useAuxiliaryContactorFeedback) {
        return appliedBits_;
    }

    std::uint16_t result = 0;
    for (std::uint8_t index = 0; index < 4; ++index) {
        if (digitalRead(config_.contactorFeedbackPins[index]) == HIGH) {
            result |= static_cast<std::uint16_t>(1u << index);
        }
    }
    return result;
}

bool RelayController::isValidCommand(std::uint16_t command) noexcept {
    using namespace register_map;

    if ((command & ~ContactorMask) != 0) {
        return false;
    }

    const bool microinverterAc = (command & K1Bit) != 0;
    const bool microinverterDc = (command & K2Bit) != 0;
    const bool mpptDc = (command & K3Bit) != 0;

    if (microinverterAc != microinverterDc) {
        return false;
    }

    return !(microinverterDc && mpptDc);
}

} // namespace opta
} // namespace solar
