#pragma once

#include "GatewayConfig.hpp"
#include "MpptRtuClient.hpp"

#include <ArduinoModbus.h>
#include <Ethernet.h>

#include <cstdint>

namespace solar {
namespace opta {

struct IpcCommand final {
    std::uint16_t requestedContactors{0};
    std::uint16_t heartbeat{0};
    std::uint16_t sequence{0};
};

class ModbusTcpGateway final {
public:
    explicit ModbusTcpGateway(const GatewayConfig& config);

    bool begin();
    void service();
    bool readIpcCommand(IpcCommand& command);
    void publish(
        const MpptSnapshot& mppt,
        std::uint16_t contactorFeedback,
        std::uint16_t faultStatus,
        std::uint16_t rtuErrorCount,
        bool heartbeatFresh);

    [[nodiscard]] bool ipcConnected() const noexcept { return ipcConnected_; }

private:
    [[nodiscard]] bool allowedIpcClient(EthernetClient& client) const;
    [[nodiscard]] std::uint16_t readDigitalInputs() const;

    const GatewayConfig& config_;
    EthernetServer tcpServer_;
    ModbusTCPServer modbusTcpServer_;
    EthernetClient activeIpcClient_;
    bool ipcConnected_{false};
};

} // namespace opta
} // namespace solar
