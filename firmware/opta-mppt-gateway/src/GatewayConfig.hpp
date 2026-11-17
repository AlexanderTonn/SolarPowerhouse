#pragma once

#include <Arduino.h>
#include <IPAddress.h>

#include <cstdint>

namespace solar {
namespace opta {

struct GatewayConfig final {
    std::uint8_t mpptSlaveId;
    unsigned long mpptBaudRate;
    std::uint32_t mpptPollIntervalMs;
    unsigned long mpptResponseTimeoutMs;

    byte mac[6];
    IPAddress localIp;
    IPAddress dns;
    IPAddress gateway;
    IPAddress subnet;

    bool enforceIpcSourceIp;
    IPAddress ipcIp;
    std::uint16_t modbusTcpPort;
    std::uint32_t ipcHeartbeatTimeoutMs;

    std::uint16_t mpptSystemInformationAddress;
    std::uint16_t mpptSystemInformationCount;
    std::uint16_t mpptSystemDataAddress;
    std::uint16_t mpptRegisterCount;

    bool useAuxiliaryContactorFeedback;
    std::uint8_t relayPins[4];
    std::uint8_t contactorFeedbackPins[4];
    std::uint8_t digitalInputPins[8];

    GatewayConfig();
};

} // namespace opta
} // namespace solar
