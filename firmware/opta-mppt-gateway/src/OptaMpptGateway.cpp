#include "OptaMpptGateway.hpp"

#include "ModbusRegisterMap.hpp"

#include <Arduino.h>

namespace solar {
namespace opta {

OptaMpptGateway::OptaMpptGateway()
    : config_(),
      relayController_(config_),
      mpptClient_(config_),
      modbusGateway_(config_) {}

void OptaMpptGateway::setup() {
    Serial.begin(115200);
    relayController_.begin();

    if (!modbusGateway_.begin()) {
        Serial.println("Failed to start Modbus TCP gateway");
        ready_ = false;
        relayController_.allOff();
        return;
    }

    if (!mpptClient_.begin()) {
        Serial.println("MPPT Modbus RTU unavailable; gateway will retry");
        faultStatus_ |= fault_bits::MpptCommunication;
    }

    lastMpptPollAt_ = millis() - config_.mpptPollIntervalMs;
    ready_ = true;

    Serial.println("Solar Powerhouse Opta MPPT gateway started");
    Serial.print("Modbus TCP address: ");
    Serial.println(config_.localIp);
}

void OptaMpptGateway::loop() {
    if (!ready_) {
        relayController_.allOff();
        delay(1000);
        return;
    }

    const auto now = millis();
    modbusGateway_.service();
    processIpcCommand(now);
    serviceIpcWatchdog(now);
    pollMppt(now);
    publish(now);
}

void OptaMpptGateway::processIpcCommand(std::uint32_t now) {
    IpcCommand command;
    if (!modbusGateway_.readIpcCommand(command)) {
        return;
    }

    bool heartbeatChanged = false;
    if (!heartbeatInitialized_) {
        heartbeatInitialized_ = true;
        lastHeartbeat_ = command.heartbeat;
        lastHeartbeatAt_ = now;
        heartbeatChanged = true;
    } else if (command.heartbeat != lastHeartbeat_) {
        lastHeartbeat_ = command.heartbeat;
        lastHeartbeatAt_ = now;
        heartbeatChanged = true;
    }

    if (heartbeatFresh(now)
        && (!commandApplied_
            || command.sequence != appliedCommandSequence_
            || heartbeatChanged)) {
        const auto applied = relayController_.applyCommand(command.requestedContactors);
        if (applied) {
            faultStatus_ &= static_cast<std::uint16_t>(
                ~(fault_bits::InvalidContactorCommand | fault_bits::InterlockViolation));
        } else {
            faultStatus_ |= fault_bits::InvalidContactorCommand
                | fault_bits::InterlockViolation;
        }
        appliedCommandSequence_ = command.sequence;
        commandApplied_ = true;
    }
}

void OptaMpptGateway::serviceIpcWatchdog(std::uint32_t now) {
    if (!heartbeatFresh(now)) {
        relayController_.allOff();
        commandApplied_ = false;
        faultStatus_ |= fault_bits::IpcHeartbeatExpired;
        return;
    }
    faultStatus_ &= static_cast<std::uint16_t>(~fault_bits::IpcHeartbeatExpired);
}

void OptaMpptGateway::pollMppt(std::uint32_t now) {
    if (!elapsed(now, lastMpptPollAt_, config_.mpptPollIntervalMs)) {
        return;
    }
    lastMpptPollAt_ = now;

    if (!mpptClient_.started() && !mpptClient_.begin()) {
        faultStatus_ |= fault_bits::MpptCommunication;
        return;
    }

    if (!mpptClient_.poll()) {
        faultStatus_ |= fault_bits::MpptCommunication;
        return;
    }

    faultStatus_ &= static_cast<std::uint16_t>(~fault_bits::MpptCommunication);
    if (mpptClient_.snapshot().faultRaw != 0) {
        faultStatus_ |= fault_bits::MpptReportedFault;
    } else {
        faultStatus_ &= static_cast<std::uint16_t>(~fault_bits::MpptReportedFault);
    }
}

void OptaMpptGateway::publish(std::uint32_t now) {
    if (!elapsed(now, lastPublishedAt_, 100)) {
        return;
    }
    lastPublishedAt_ = now;
    modbusGateway_.publish(
        mpptClient_.snapshot(),
        relayController_.feedbackBits(),
        faultStatus_,
        mpptClient_.errorCount(),
        heartbeatFresh(now));
}

bool OptaMpptGateway::heartbeatFresh(std::uint32_t now) const {
    return modbusGateway_.ipcConnected()
        && heartbeatInitialized_
        && !elapsed(now, lastHeartbeatAt_, config_.ipcHeartbeatTimeoutMs);
}

bool OptaMpptGateway::elapsed(
    std::uint32_t now,
    std::uint32_t since,
    std::uint32_t interval) noexcept {
    return static_cast<std::uint32_t>(now - since) >= interval;
}

OptaMpptGateway& gateway() {
    static OptaMpptGateway instance;
    return instance;
}

} // namespace opta
} // namespace solar
