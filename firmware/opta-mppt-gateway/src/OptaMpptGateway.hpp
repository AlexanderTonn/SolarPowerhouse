#pragma once

#include "GatewayConfig.hpp"
#include "ModbusTcpGateway.hpp"
#include "MpptRtuClient.hpp"
#include "RelayController.hpp"

#include <cstdint>

namespace solar {
namespace opta {

class OptaMpptGateway final {
public:
    OptaMpptGateway();

    void setup();
    void loop();

private:
    void processIpcCommand(std::uint32_t now);
    void serviceIpcWatchdog(std::uint32_t now);
    void pollMppt(std::uint32_t now);
    void publish(std::uint32_t now);
    [[nodiscard]] bool heartbeatFresh(std::uint32_t now) const;
    [[nodiscard]] static bool elapsed(
        std::uint32_t now,
        std::uint32_t since,
        std::uint32_t interval) noexcept;

    GatewayConfig config_;
    RelayController relayController_;
    MpptRtuClient mpptClient_;
    ModbusTcpGateway modbusGateway_;

    std::uint16_t appliedCommandSequence_{0};
    std::uint16_t lastHeartbeat_{0};
    std::uint16_t faultStatus_{0};
    std::uint32_t lastHeartbeatAt_{0};
    std::uint32_t lastMpptPollAt_{0};
    std::uint32_t lastPublishedAt_{0};
    bool heartbeatInitialized_{false};
    bool commandApplied_{false};
    bool ready_{false};
};

OptaMpptGateway& gateway();

} // namespace opta
} // namespace solar
